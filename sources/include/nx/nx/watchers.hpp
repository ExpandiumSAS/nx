#ifndef __NX_WATCHERS_H__
#define __NX_WATCHERS_H__

#include <nx/config.h>
#include <nx/event.hpp>
#include <nx/watcher_base.hpp>

namespace nx {

class NX_API io : public watcher_base<io, ev_io>
{
public:
    using base_type = watcher_base<io, ev_io>;
    using cb_type = typename base_type::cb_type;

    io(int fd, event e, cb_type cb)
};

} // namespace nx

#endif // __NX_WATCHERS_H__
