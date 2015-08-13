#ifndef __NX_SOCKET_H__
#define __NX_SOCKET_H__

#include <memory>
#include <mutex>
#include <atomic>

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
    { read(); }

    virtual void stop()
    {
        stop_ = true;
        close();
    }

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
        with_queue([&](auto& q) { q.emplace(b, e); });
        write();

        return *this;
    }

    template <typename T>
    this_type& push_write(T&& t)
    {
        with_queue(
            [this,t = std::move(t)](auto& q) {
                q.emplace(std::move(t));
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

    void close()
    {
        std::lock_guard<std::mutex> lock(wqm_);

        if (!writing_ && socket_.is_open()) {
            closed_ = true;

            error_code ec;

            socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            socket_.close();
            base_type::handler(tags::on_close)(derived());
        }
    }

private:
    template <typename Callback>
    auto with_queue(Callback cb)
    { return with(wq_, wqm_, cb); }

    template <typename Object, typename Callback>
    auto with(Object& o, std::mutex& m, Callback cb)
    {
        std::lock_guard<std::mutex> lock(m);

        return cb(o);
    }

    void read()
    {
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

                if (stop_) {
                    close();
                } else {
                    read();
                }
            }
        );
    }

    void write()
    {
        std::lock_guard<std::mutex> lock(wqm_);

        if (writing_ || closed_) {
            return;
        }

        if (wq_.empty()) {
            io_service().post(
                [this]() {
                    base_type::handler(tags::on_drain)(derived());
                }
            );

            return;
        }

        writing_ = true;
        buffer& b = wq_.front();

        asio::async_write(
            socket_,
            asio::buffer(b),
            [this](const error_code& ec, std::size_t count) {
                writing_ = false;

                if (handle_error(derived(), "write", ec)) {
                    return;
                }

                with_queue([](auto& q) { q.pop(); });

                if (stop_) {
                    close();
                } else {
                    write();
                }
            }
        );
    }

    socket_type socket_;
    buffer buf_;
    buffer rbuf_;
    std::atomic_bool writing_ = { false };
    std::atomic_bool stop_ = { false };
    std::atomic_bool closed_ = { false };
    buffer_queue wq_;
    std::mutex wqm_;
};

} // namespace nx

#endif // __NX_SOCKET_H__
