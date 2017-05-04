#ifndef __NX_SOCKET_TEMPLATE_FUNCTIONS__
#define __NX_SOCKET_TEMPLATE_FUNCTIONS__

namespace nx {

template <typename Socket, typename Connected>
Socket&
connect(
    Socket& s,
    const typename Socket::endpoint_type& to,
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
    const typename Socket::endpoint_type& to,
    Connected cb
)
{
    auto p = new_object<Socket>();
    auto& s = *p;

    return connect(s, to, cb);
}

// maybe this function should be in tcp.hpp because only tcp has a resolver
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
typename Socket::endpoint_type
serve(
    Socket& s,
    const typename Socket::endpoint_type& from,
    Accepted accept_cb,
    Read read_cb
)
{
    auto& a = s.make_acceptor();

    a.open(from.protocol());
    s.reuse_addr(a, from);
    a.bind(from);
    a.listen();

    accept(s, accept_cb, read_cb);

    return a.local_endpoint();
}

template <typename Socket, typename Accepted, typename Read>
typename Socket::endpoint_type
serve(
    const typename Socket::endpoint_type& from,
    Accepted accept_cb,
    Read read_cb
)
{
    auto p = new_object<Socket>();

    return serve(*p, from, accept_cb, read_cb);
}

} // namespace nx

#endif // __NX_SOCKET_TEMPLATE_FUNCTIONS__