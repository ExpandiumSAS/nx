#ifndef __NX_ENDPOINT_H__
#define __NX_ENDPOINT_H__

#include <stdint.h>

#include <boost/asio.hpp>

namespace nx {

namespace asio = boost::asio;

using endpoint_tcp = asio::ip::tcp::endpoint;
using endpoint_local = asio::local::stream_protocol::endpoint;

inline
endpoint_tcp
make_endpoint_tcp(const std::string& address, uint16_t port = 0)
{
    return
        endpoint_tcp(
            asio::ip::address::from_string(address),
            port
        );
}

inline
endpoint_local
make_endpoint_local(const std::string& path)
{
    return
        endpoint_local(
            path
        );
}

struct NX_API endpoint_generic
{
    enum protocol
    {
        TCP,
        LOCAL
    };

    endpoint_generic(const std::string& addr, uint16_t port = 0)
    {
        if (addr[0] == '/')
        {
            ep_local = make_endpoint_local(addr);
            ep_protocol = LOCAL;
        }
        else
        {
            ep_protocol = TCP;
            ep_tcp = make_endpoint_tcp(addr, port);
        }
    }

    endpoint_generic(const endpoint_tcp& ep)
    {
        ep_protocol = TCP;
        ep_tcp = ep;
    }

    endpoint_generic(const endpoint_local& ep)
    {
        ep_protocol = LOCAL;
        ep_local = ep;
    }

    protocol ep_protocol;
    endpoint_tcp ep_tcp;
    endpoint_local ep_local;
};

inline
endpoint_generic
make_endpoint(const std::string &address, uint16_t port = 0)
{
    endpoint_generic ep(address, port);
    return (ep);
}

} // namespace nx

#endif // __NX_ENDPOINT_H__
