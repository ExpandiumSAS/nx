#include <nx/handle.hpp>

namespace nx {

handle::handle(int fh)
: fh_(fh)
{}

handle::~handle()
{}

} // namespace nx
