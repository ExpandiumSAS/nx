#ifndef __NX_WATCHER_BASE_H__
#define __NX_WATCHER_BASE_H__

#include <memory>
#include <unordered_set>

#include <nx/config.h>
#include <nx/ev.hpp>

namespace nx {

class NX_API watcher_base : public std::enable_shared_from_this<watcher_base>
{
public:
    std::shared_ptr<watcher_base> ptr();

    virtual void start() noexcept = 0;
    virtual void stop() noexcept = 0;
    virtual void stop(void_cb&& on_stopped) noexcept = 0;
};

using watcher_ptr = std::shared_ptr<watcher_base>;
using watcher_set = std::unordered_set<watcher_ptr>;

} // namespace nx

#endif // __NX_WATCHER_BASE_H__
