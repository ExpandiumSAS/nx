#ifndef __NX_SOCKET_H__
#define __NX_SOCKET_H__

#include <fcntl.h>

#if defined(LINUX)
#include <sys/sendfile.h>
#elif defined(DARWIN)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#endif

#include <memory>
#include <mutex>
#include <queue>

#include <boost/asio.hpp>

#include <nx/config.h>
#include <nx/object.hpp>
#include <nx/endpoint.hpp>
#include <nx/service.hpp>
#include <nx/buffer.hpp>
#include <nx/file.hpp>

namespace nx {

namespace asio = boost::asio;

namespace tags {

struct on_read_tag : callback_tag {};
struct on_drain_tag : callback_tag {};
struct on_close_tag : callback_tag {};

const on_read_tag on_read = {};
const on_drain_tag on_drain = {};
const on_close_tag on_close = {};

} // namespace tags

enum class write_cmd
{
    buffer,
    file
};

using write_cmd_queue = std::queue<write_cmd>;

template <
    typename Derived,
    typename Socket,
    typename... Callbacks
>
class socket
: public object<
    Derived,
    callback<tags::on_read_tag, Derived&>,
    callback<tags::on_drain_tag, Derived&>,
    callback<tags::on_close_tag, Derived&>,
    Callbacks...
>
{
public:
    using this_type = socket<Derived, Socket, Callbacks...>;
    using base_type = object<
        Derived,
        callback<tags::on_read_tag, Derived&>,
        callback<tags::on_drain_tag, Derived&>,
        callback<tags::on_close_tag, Derived&>,
        Callbacks...
    >;
    using socket_type = Socket;

    static constexpr std::size_t default_read_size = 10 * 1024 * 1024;

    socket()
    : socket_(service::get().io_service())
    {}

    socket(socket_type other_socket)
    : socket_(std::move(other_socket))
    {}

    socket(const socket& other) = delete;
    socket(socket&& other) = default;
    socket& operator=(const socket& other) = delete;
    socket& operator=(socket&& other) = default;

    virtual ~socket()
    {}

    socket_type& sock()
    { return socket_; }

    const socket_type& sock() const
    { return socket_; }

    virtual void start()
    { base_type::postpone() << [this]() { read(); }; }

    virtual void stop()
    { close_after_write(); }

    this_type&
    operator<<(const char* s)
    { return push_write(s, s + std::strlen(s)); }

    this_type&
    operator<<(const std::string& s)
    { return push_write(s.begin(), s.end()); }

    this_type&
    operator<<(std::string&& s)
    {
        return
            push_write(
                std::make_move_iterator(s.begin()),
                std::make_move_iterator(s.end())
            );
    }

    this_type&
    operator>>(std::string& s)
    {
        s.insert(s.end(), rbuf_.begin(), rbuf_.end());
        rbuf_.clear();

        return *this;
    }

    this_type&
    operator<<(buffer&& b)
    { return push_write(std::move(b)); }

    this_type&
    operator>>(buffer& b)
    {
        b.insert(b.end(), rbuf_.begin(), rbuf_.end());
        rbuf_.clear();

        return *this;
    }

    this_type&
    operator<<(const nx::file& f)
    { return push_write(f); }

protected:
    this_type& push_write(const file& f)
    {
        return start_write(
            [&]() {
                wcq_.emplace(write_cmd::file);
                fq_.emplace(f);
            }
        );
    }

    template <typename Iterator>
    this_type& push_write(Iterator b, Iterator e)
    {
        return start_write(
            [&]() {
                wcq_.emplace(write_cmd::buffer);
                bq_.emplace(b, e);
            }
        );
    }

    template <typename T>
    this_type& push_write(T&& t)
    {
        return start_write(
            [this,t = std::move(t)]() {
                wcq_.emplace(write_cmd::buffer);
                bq_.emplace(std::move(t));
            }
        );
    }

    template <typename Callable>
    this_type& start_write(Callable cb)
    {
        locked(cb);
        write();

        return *this;
    }

    Derived& derived()
    { return *static_cast<Derived* const>(this); }
    Derived const& derived() const
    { return *static_cast<Derived const*>(this); }

    auto& io_service()
    { return socket_.get_io_service(); }

    buffer& rbuf()
    { return rbuf_; }

    const buffer& rbuf() const
    { return rbuf_; }

    void close_after_write()
    {
        std::lock_guard<std::mutex> lock(m_);

        stop_ = true;

        if (!closed_ && !writing_) {
            base_type::postpone() << [this]() { close(); };
        }
    }

    void close()
    {
        std::lock_guard<std::mutex> lock(m_);

        if (!closed_ && socket_.is_open()) {
            error_code ec;

            socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            socket_.close();
            closed_ = true;
            base_type::handler(tags::on_close)(derived());
            base_type::dispose();
        }
    }

private:
    template <typename Callback>
    auto locked(Callback cb)
    {
        std::lock_guard<std::mutex> lock(m_);

        return cb();
    }

    void read()
    {
        std::lock_guard<std::mutex> lock(m_);

        if (stop_ || closed_) {
            return;
        }

        buf_.resize(default_read_size);

        socket_.async_read_some(
            asio::buffer(buf_),
            [this](const error_code& ec, std::size_t count) {
                if (handle_error(derived(), "read", ec)) {
                    return;
                }

                if (count > 0) {
                    rbuf_.insert(
                        rbuf_.end(),
                        buf_.begin(), buf_.begin() + count
                    );

                    buf_.clear();
                    base_type::handler(tags::on_read)(derived());
                }

                base_type::postpone() << [this]() { read(); };
            }
        );
    }

    void write()
    {
        std::lock_guard<std::mutex> lock(m_);

        if (writing_ || stop_ || closed_) {
            return;
        }

        if (wcq_.empty()) {
            base_type::postpone() << [this]() {
                base_type::handler(tags::on_drain)(derived());
            };

            return;
        }

        writing_ = true;

        switch (wcq_.front()) {
            case write_cmd::buffer:
            write_buffer();
            break;
            case write_cmd::file:
            write_file();
            break;
        }
    }

    void write_buffer()
    {
        buffer& b = bq_.front();

        asio::async_write(
            socket_,
            asio::buffer(b),
            [this](const error_code& ec, std::size_t count) {
                writing_ = false;

                if (stop_) {
                    stop();
                    return;
                }

                if (handle_error(derived(), "write", ec)) {
                    return;
                }

                locked([&]() { wcq_.pop(); bq_.pop(); });

                base_type::postpone() << [this](){ write(); };
            }
        );
    }

    void write_file()
    {
        file& f = fq_.front();
        f.fd = open(f.path.c_str(), O_RDONLY);

        if (f.fd == -1) {
            auto ec = error_code(
                errno,
                boost::system::system_category()
            );

            if (handle_error(derived(), "sendfile open", ec)) {
                return;
            }
        }
    }

    socket_type socket_;
    buffer buf_;
    buffer rbuf_;
    bool stop_ = false;
    bool writing_ = false;
    bool closed_ = false;
    write_cmd_queue wcq_;
    buffer_queue bq_;
    file_queue fq_;
    std::mutex m_;
};

} // namespace nx

#endif // __NX_SOCKET_H__
