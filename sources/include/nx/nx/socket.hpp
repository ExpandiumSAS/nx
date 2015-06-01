#ifndef __NX_SOCKET_H__
#define __NX_SOCKET_H__

#include <memory>

#include <boost/asio.hpp>

#include <nx/config.h>

namespace nx {

using namespace asio = boost::asio;

class service;

class NX_API socket
: public std:enable_shared_from_this<socket>
{
public:
    socket(const socket& other) = delete;
    socket& operator=(const socket& other) = delete;

    explicit socket(service& s);
    ~socket();

    void start();
    void stop();

private:
    void read();
    void write();
    void close();
    std::string desc() const;

    service& s_;
    asio::ip::tcp::socket socket_;
};

using socket_ptr = std::shared_ptr<socket>;

template <typename... Args>
socket_ptr
new_socket(Args&&... args)
{ return socket_ptr(new socket(std::forward<Args>(args)...)); }

} // namespace nx

#endif // __NX_SOCKET_H__
