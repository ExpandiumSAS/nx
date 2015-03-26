#ifndef __NX_EVENT_H__
#define __NX_EVENT_H__

#include <nx/ev.hpp>

namespace nx {

enum class event : int
{
    undef    = EV_UNDEF,
    none     = EV_NONE,
    read     = EV_READ,
    write    = EV_WRITE,
    timer    = EV_TIMER,
    periodic = EV_PERIODIC,
    signal   = EV_SIGNAL,
    child    = EV_CHILD,
    stat     = EV_STAT,
    idle     = EV_IDLE,
    check    = EV_CHECK,
    prepare  = EV_PREPARE,
    fork     = EV_FORK,
    async    = EV_ASYNC,
    embed    = EV_EMBED,
    error    = EV_ERROR
};

} // namespace nx

#endif // __NX_EVENT_H__
