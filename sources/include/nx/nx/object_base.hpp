#ifndef __NX_OBJECT_BASE_H__
#define __NX_OBJECT_BASE_H__

#include <memory>

#include <nx/config.h>

namespace nx {

class NX_API object_base
: public std::enable_shared_from_this<object_base>
{
public:
    object_base() = default;
    object_base(const object_base& other) = delete;
    object_base(object_base&& other) = default;
    object_base& operator=(const object_base& other) = delete;
    object_base& operator=(object_base&& other) = default;

    std::shared_ptr<object_base> ptr()
    {
        auto self(shared_from_this());

        return self;
    }

    virtual void stop() = 0;
};

using object_ptr = std::shared_ptr<object_base>;

} // namespace nx

#endif // __NX_OBJECT_BASE_H__
