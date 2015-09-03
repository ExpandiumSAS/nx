#ifndef __NX_SOCKET_H__
#define __NX_SOCKET_H__

#include <memory>
#include <mutex>

#include <boost/asio.hpp>

#include <nx/config.h>
#include <nx/object.hpp>
#include <nx/endpoint.hpp>
#include <nx/service.hpp>
#include <nx/buffer.hpp>

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

protected:
    template <typename Iterator>
    this_type& push_write(Iterator b, Iterator e)
    {
        locked([&]() { wq_.emplace(b, e); });
        write();

        return *this;
    }

    template <typename T>
    this_type& push_write(T&& t)
    {
        locked(
            [this,t = std::move(t)]() {
                wq_.emplace(std::move(t));
            }
        );

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

        if (wq_.empty()) {
            base_type::postpone() << [this]() {
                base_type::handler(tags::on_drain)(derived());
            };

            return;
        }

        writing_ = true;
        buffer& b = wq_.front();

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

                locked([&]() { wq_.pop(); });

                base_type::postpone() << [this](){ write(); };
            }
        );
    }

    socket_type socket_;
    buffer buf_;
    buffer rbuf_;
    bool stop_ = false;
    bool writing_ = false;
    bool closed_ = false;
    buffer_queue wq_;
    std::mutex m_;
};

} // namespace nx

#endif // __NX_SOCKET_H__
