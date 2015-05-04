#include <nx/watcher_base.hpp>

namespace nx {

std::shared_ptr<watcher_base>
watcher_base::ptr()
{ return shared_from_this(); }

} // namespace nx
