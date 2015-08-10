#ifndef __NX_SOCKET_H__
#define __NX_SOCKET_H__

#include <memory>

#include <boost/asio.hpp>

#include <nx/config.h>
#include <nx/socket_base.hpp>
#include <nx/service.hpp>
#include <nx/buffer.hpp>
#include <nx/callbacks.hpp>

namespace nx {

namespace asio = boost::asio;

namespace tags {

struct on_error_tag : callback_tag {};
struct on_read_tag : callback_tag {};
struct on_readable_tag : callback_tag {};
struct on_writable_tag : callback_tag {};
struct on_drain_tag : callback_tag {};
struct on_eof_tag : callback_tag {};
struct on_close_tag : callback_tag {};

const on_error_tag on_error = {};
const on_read_tag on_read = {};
const on_readable_tag on_readable = {};
const on_writable_tag on_writable = {};
const on_drain_tag on_drain = {};
const on_eof_tag on_eof = {};
const on_close_tag on_close = {};

} // namespace tags

using error_code = boost::system::error_code;

template <typename Socket>
bool
handle_error(Socket& s, const char* what, const error_code& e)
{
    if (!e || e == asio::error::operation_aborted) {
        return false;
    }

    bool stop = false;

    if (e == asio::error::eof) {
        stop = true;
    } else if (!s.handler(tags::on_error)(s, e)) {
        // Unhandled error
        std::cerr << "unhandled error: " << e.message() << std::endl;
        stop = true;
    }

    if (stop) {
        s.stop();
    }

    return true;
}

template <
    typename Derived,
    typename Socket,
    typename... Callbacks
>
class socket : public socket_base
{
public:
    using this_type = socket<Derived, Socket, Callbacks...>;
    using socket_type = Socket;
    using callbacks = nx::callbacks<
        callback<tags::on_error_tag, Derived&, const error_code&>,
        callback<tags::on_read_tag, Derived&>,
        callback<tags::on_readable_tag, Derived&>,
        callback<tags::on_writable_tag, Derived&>,
        callback<tags::on_drain_tag, Derived&>,
        callback<tags::on_eof_tag, Derived&>,
        callback<tags::on_close_tag, Derived&>,
        Callbacks...
    >;

    static constexpr std::size_t default_read_size = 10 * 1024 * 1024;

    socket()
    : socket_(service::get().io_service())
    {}

    socket(const socket& other) = delete;
    socket& operator=(const socket& other) = delete;

    virtual ~socket()
    {}

    template <
        typename Tag,
        typename Enabled = typename std::enable_if<
            std::is_base_of<callback_tag, Tag>::value
        >::type
    >
    auto&
    handler(const Tag& t)
    { return callbacks_.get(t); }

    template <typename Tag>
    auto&
    operator[](const Tag& t)
    { return handler(t); }

    void start()
    {
        service::get().add(shared_from_this());
        read();
    }

    void stop()
    {
        service::get().remove(shared_from_this());
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
        wq_.emplace(b, e);
        write();

        return *this;
    }

    template <typename T>
    this_type& push_write(T&& t)
    {
        wq_.emplace(std::move(t));
        write();

        return *this;
    }

    Derived& derived()
    { return *static_cast<Derived* const>(this); }
    Derived const& derived() const
    { return *static_cast<Derived const*>(this); }

    socket_type& sock()
    { return socket_; }

    const socket_type& sock() const
    { return socket_; }

private:
    void read()
    {
        buf_.resize(default_read_size);

        auto self(shared_from_this());

        socket_.async_read_some(
            asio::buffer(buf_),
            [this,self](const error_code& ec, std::size_t count) {
                if (handle_error(derived(), "read", ec)) {
                    return;
                }

                rbuf_.insert(rbuf_.end(), buf_.begin(), buf_.end());
                buf_.clear();

                handler(tags::on_read)(derived());
                read();
            }
        );
    }

    void write()
    {
        if (wq_.empty()) {
            handler(tags::on_drain)(derived());
            return;
        }

        auto self(shared_from_this());
        buffer& b = wq_.front();

        asio::async_write(
            socket_,
            asio::buffer(b),
            [this,self](const error_code& ec, std::size_t count) {
                if (handle_error(derived(), "write", ec)) {
                    return;
                }

                wq_.pop();
                write();
            }
        );
    }

    void close();
    std::string desc() const;

    socket_type socket_;
    callbacks callbacks_;
    buffer buf_;
    buffer rbuf_;
    buffer_queue wq_;
};

template <typename Derived, typename... Args>
std::shared_ptr<Derived>
new_socket(Args&&... args)
{ return std::make_shared<Derived>(std::forward<Args>(args)...); }

} // namespace nx

#endif // __NX_SOCKET_H__
