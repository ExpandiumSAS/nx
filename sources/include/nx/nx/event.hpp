#ifndef __NX_EVENT_H__
#define __NX_EVENT_H__

#include <nx/ev.hpp>

namespace nx {

enum
{
    NX_UNDEF    = EV_UNDEF,
    NX_NONE     = EV_NONE,
    NX_READ     = EV_READ,
    NX_WRITE    = EV_WRITE,
    NX_TIMER    = EV_TIMER,
    NX_PERIODIC = EV_PERIODIC,
    NX_SIGNAL   = EV_SIGNAL,
    NX_CHILD    = EV_CHILD,
    NX_STAT     = EV_STAT,
    NX_IDLE     = EV_IDLE,
    NX_CHECK    = EV_CHECK,
    NX_PREPARE  = EV_PREPARE,
    NX_FORK     = EV_FORK,
    NX_ASYNC    = EV_ASYNC,
    NX_EMBED    = EV_EMBED,
    NX_ERROR    = EV_ERROR
};

} // namespace nx

#endif // __NX_EVENT_H__
