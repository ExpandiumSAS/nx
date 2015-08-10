#ifndef __NX_TCP_H__
#define __NX_TCP_H__

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

    auto local() const
    { return base_type::sock().local_endpoint(); }

    auto remote() const
    { return base_type::sock().remote_endpoint(); }
};

class tcp : public tcp_base<tcp>
{
    using base_type = tcp_base<tcp>;

    using base_type::tcp_base;
};

} // namespace nx

#endif // __NX_TCP_H__
