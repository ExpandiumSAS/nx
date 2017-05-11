#ifndef __NX_ENDPOINT_H__
#define __NX_ENDPOINT_H__

#include <stdint.h>
#include <fstream>

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

struct NX_API endpoint
{
    enum protocol
    {
        UNINITIALIZED,
        TCP,
        LOCAL
    };

    endpoint() = default;

    endpoint(const std::string& addr, uint16_t port = 0)
    {
        if (addr[0] == '/' ||
            addr[0] == '~' ){
            ep_local = make_endpoint_local(addr);
            ep_protocol = LOCAL;
        } else {
            ep_protocol = TCP;
            ep_tcp = make_endpoint_tcp(addr, port);
        }
    }

    endpoint(const endpoint_tcp& ep)
    {
        ep_protocol = TCP;
        ep_tcp = ep;
    }

    endpoint(const endpoint_local& ep)
    {
        ep_protocol = LOCAL;
        ep_local = ep;
    }

    bool operator==(const endpoint& other)
    {
        if (ep_protocol != other.ep_protocol) {
            return false;
        }

        if (ep_protocol == LOCAL) {
            return ep_local == other.ep_local;
        } else {
            return ep_tcp == other.ep_tcp;
        }
    }

    bool operator!=(const endpoint& other)
    {
        return !(*this == other);
    }

    protocol ep_protocol = UNINITIALIZED;
    endpoint_tcp ep_tcp;
    endpoint_local ep_local;
};

inline
std::ostream&
operator<<(std::ostream& os, const endpoint& ep)
{
    if (ep.ep_protocol == endpoint::protocol::LOCAL){
        os << ep.ep_local;
    } else {
        os << ep.ep_tcp;
    }
    return os;
}

inline
endpoint
make_endpoint(const std::string& address, uint16_t port = 0)
{
    return
        endpoint(
                    address,
                    port
        );
}

} // namespace nx

#endif // __NX_ENDPOINT_H__
