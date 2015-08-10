#ifndef __NX_SOCKET_BASE_H__
#define __NX_SOCKET_BASE_H__

#include <memory>

#include <nx/config.h>
#include <nx/buffer.hpp>

namespace nx {

class NX_API socket_base
: public std::enable_shared_from_this<socket_base>
{
public:
    socket_base(const socket_base& other) = delete;
    socket_base& operator=(const socket_base& other) = delete;

    std::shared_ptr<socket_base> ptr();

    void start();
    void stop();

    virtual buffer& rbuf() = 0;
    virtual const buffer& rbuf() const = 0;
};

using socket_ptr = std::shared_ptr<socket_base>;

} // namespace nx

#endif // __NX_SOCKET_BASE_H__
