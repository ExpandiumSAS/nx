#include <nx/handle_base.hpp>

namespace nx {

std::shared_ptr<handle_base>
handle_base::ptr()
{ return shared_from_this(); }

} // namespace nx
