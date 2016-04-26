#ifndef __NX_HANDLERS_H__
#define __NX_HANDLERS_H__

#include <functional>
#include <vector>

namespace nx {

using void_cb = std::function<void()>;
using void_cbs = std::vector<void_cb>;

} // namespace nx

#endif // __NX_HANDLERS_H__
