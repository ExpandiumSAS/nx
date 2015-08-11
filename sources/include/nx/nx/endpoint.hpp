#ifndef __NX_ENDPOINT_H__
#define __NX_ENDPOINT_H__

#include <stdint.h>

#include <boost/asio.hpp>

namespace nx {

namespace asio = boost::asio;

using endpoint = asio::ip::tcp::endpoint;

inline
endpoint
make_endpoint(const std::string& address, uint16_t port)
{
    return
        endpoint(
            asio::ip::address::from_string(address),
            port
        );
}

} // namespace nx

#endif // __NX_ENDPOINT_H__
