#ifndef __NX_TCP_H__
#define __NX_TCP_H__

#include <sstream>

#include <nx/socket.hpp>

namespace nx {

template <typename Derived, typename... Callbacks>
class tcp_base
: public socket<
    Derived,
    asio::ip::tcp::socket,
    Callbacks...
>
{
public:
    using base_type = socket<
        Derived,
        asio::ip::tcp::socket,
        Callbacks...
    >;

    using acceptor_type = asio::ip::tcp::acceptor;
    using resolver_type = asio::ip::tcp::resolver;

    using base_type::base_type;

    tcp_base() = default;
    tcp_base(const tcp_base& other) = delete;
    tcp_base(tcp_base&& other) = default;
    tcp_base& operator=(const tcp_base& other) = delete;
    tcp_base& operator=(tcp_base&& other) = default;

    auto local() const
    { return base_type::sock().local_endpoint(); }

    std::string local_str() const
    {
        std::ostringstream oss;

        oss << local();

        return oss.str();
    }

    auto remote() const
    { return base_type::sock().remote_endpoint(); }

    std::string remote_str() const
    {
        std::ostringstream oss;

        oss << remote();

        return oss.str();
    }

    acceptor_type& make_acceptor()
    {
        acceptor_ptr_ = std::make_unique<acceptor_type>(
            base_type::io_service()
        );

        return *acceptor_ptr_;
    }

    acceptor_type& acceptor()
    { return *acceptor_ptr_; }

    const acceptor_type& acceptor() const
    { return *acceptor_ptr_; }

    resolver_type& make_resolver()
    {
        resolver_ptr_ = std::make_unique<resolver_type>(
            base_type::io_service()
        );

        return *resolver_ptr_;
    }

    resolver_type& resolver()
    { return *resolver_ptr_; }

    const resolver_type& resolver() const
    { return *resolver_ptr_; }

private:
    using acceptor_ptr = std::unique_ptr<acceptor_type>;
    using resolver_ptr = std::unique_ptr<resolver_type>;

    acceptor_ptr acceptor_ptr_;
    resolver_ptr resolver_ptr_;
};

class tcp : public tcp_base<tcp>
{
    using base_type = tcp_base<tcp>;

    using base_type::tcp_base;
};

template <typename Socket, typename Connected>
Socket&
connect(
    Socket& s,
    const endpoint& to,
    Connected cb
)
{
    s.sock().async_connect(
        to,
        [&s,cb](const error_code& ec) {
            if (handle_error(s, "connect", ec)) {
                return;
            }

            cb(s);
            s.start();
        }
    );

    return s;
}

template <typename Socket, typename Connected>
Socket&
connect(
    const endpoint& to,
    Connected cb
)
{
    auto p = new_object<Socket>();
    auto& s = *p;

    return connect(s, to, cb);
}

template <typename Socket, typename Connected>
Socket&
connect(
    const std::string& host,
    const std::string& port,
    Connected cb
)
{
    using iterator = typename Socket::resolver_type::iterator;

    auto p = new_object<Socket>();
    auto& s = *p;

    auto& r = s.make_resolver();

    r.async_resolve(
        { host, port },
        [&s,cb](const error_code& ec, iterator eit) {
            if (handle_error(s, "resolve", ec)) {
                return;
            }

            auto to = *eit;
            ++eit;

            // Prepare for connect error
            s[tags::on_error] =
                [eit, cb](auto& s, const error_code& ec) mutable {
                    bool handled = false;

                    if (eit != iterator()) {
                        // Connect failed, try next endpoint
                        handled = true;
                        s.sock().close();
                        auto to = *eit;
                        ++eit;
                        connect(s, to, cb);
                    }

                    return handled;
                };

            // Connect to first endpoint
            connect(s, to, cb);
        }
    );

    return s;
}

template <typename Socket, typename Accepted, typename Read>
void
accept(Socket& s, Accepted accept_cb, Read read_cb)
{
    auto& a = s.acceptor();

    a.async_accept(
        s.sock(),
        [&s,accept_cb,read_cb](const error_code& ec) {
            if (!s.acceptor().is_open()) {
                return;
            }

            if (handle_error(s, "accept", ec)) {
                return;
            }

            auto cs_ptr = new_object<Socket>(std::move(s.sock()));
            auto& cs = *cs_ptr;

            cs.sock().non_blocking();
            cs[tags::on_read] = read_cb;
            cs.start();
            accept_cb(cs);

            accept(s, accept_cb, read_cb);
        }
    );
}

template <typename Socket, typename Accepted, typename Read>
endpoint
serve(
    Socket& s,
    const endpoint& from,
    Accepted accept_cb,
    Read read_cb
)
{
    auto& a = s.make_acceptor();

    a.open(from.protocol());
    a.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    a.bind(from);
    a.listen();

    accept(s, accept_cb, read_cb);

    return a.local_endpoint();
}

template <typename Socket, typename Accepted, typename Read>
endpoint
serve(
    const endpoint& from,
    Accepted accept_cb,
    Read read_cb
)
{
    auto p = new_object<Socket>();

    return serve(*p, from, accept_cb, read_cb);
}

} // namespace nx

#endif // __NX_TCP_H__
