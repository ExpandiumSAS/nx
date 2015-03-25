#ifndef __NX_CALLBACK_ACCESS_H__
#define __NX_CALLBACK_ACCESS_H__

#include <utility>

namespace nx {

struct callback_access
{
    template <typename Tag, typename Class, typename... Args>
    static
    void
    call(Class& c, Args&&... args)
    { c(Tag(), std::forward<Args>(args)...); }

    template <typename Class, typename... Args>
    static
    void
    call(Class& c, Args&&... args)
    { c(std::forward<Args>(args)...); }

private:
    callback_access();
};

} // namespace nx

#endif // __NX_CALLBACK_ACCESS_H__
