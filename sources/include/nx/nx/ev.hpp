#ifndef __NX_EV_H__
#define __NX_EV_H__

#ifdef EV_ERROR
#undef EV_ERROR
#endif

#include <ev.h>

#include <functional>

namespace nx {

using evloop = struct ev_loop*;
using timestamp = ev_tstamp;

using loop_cb = std::function<void(evloop)>;
using void_cb = std::function<void()>;

} // namespace nx

#endif // __NX_EV_H__
