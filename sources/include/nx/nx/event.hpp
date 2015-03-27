#ifndef __NX_EVENT_H__
#define __NX_EVENT_H__

#include <nx/ev.hpp>

namespace nx {

enum
{
    UNDEF    = EV_UNDEF,
    NONE     = EV_NONE,
    READ     = EV_READ,
    WRITE    = EV_WRITE,
    TIMER    = EV_TIMER,
    PERIODIC = EV_PERIODIC,
    SIGNAL   = EV_SIGNAL,
    CHILD    = EV_CHILD,
    STAT     = EV_STAT,
    IDLE     = EV_IDLE,
    CHECK    = EV_CHECK,
    PREPARE  = EV_PREPARE,
    FORK     = EV_FORK,
    ASYNC    = EV_ASYNC,
    EMBED    = EV_EMBED,
    ERROR    = EV_ERROR
};

} // namespace nx

#endif // __NX_EVENT_H__
