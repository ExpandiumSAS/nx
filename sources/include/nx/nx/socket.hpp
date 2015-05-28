#ifndef __NX_SOCKET_H__
#define __NX_SOCKET_H__

#include <memory>

#include <boost/asio.hpp>

#include <nx/config.h>

namespace nx {

using namespace asio = boost::asio;

class NX_API socket
: public std:enable_shared_from_this<socket>
{

private:
    asio::ip::tcp::socket socket_;
};

} // namespace nx

#endif // __NX_SOCKET_H__
