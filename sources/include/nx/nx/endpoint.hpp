#ifndef __NX_ENDPOINT_H__
#define __NX_ENDPOINT_H__

#include <stdint.h>

#include <boost/asio.hpp>

namespace nx {

namespace asio = boost::asio;

using endpoint = asio::ip::tcp::endpoint;
using endpoint_local = asio::local::stream_protocol::endpoint;

inline
endpoint
make_endpoint(const std::string& address, uint16_t port = 0)
{
    return
        endpoint(
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


} // namespace nx

#endif // __NX_ENDPOINT_H__
