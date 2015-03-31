#ifndef __NX_HANDLE_BASE_H__
#define __NX_HANDLE_BASE_H__

#include <memory>
#include <unordered_set>

#include <nx/config.h>
#include <nx/buffer.hpp>

namespace nx {

class NX_API handle_base : public std::enable_shared_from_this<handle_base>
{
public:
    std::shared_ptr<handle_base> ptr();

    virtual void push_close() = 0;

    virtual void set_nonblocking() = 0;

    virtual int fh() const = 0;

    virtual buffer& rbuf() = 0;
    virtual const buffer& rbuf() const = 0;
};

using handle_ptr = std::shared_ptr<handle_base>;
using handle_set = std::unordered_set<handle_ptr>;

} // namespace nx

#endif // __NX_HANDLE_BASE_H__
