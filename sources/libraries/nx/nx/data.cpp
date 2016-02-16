#include <nx/data.hpp>

namespace nx {

bool
data::prev_is_stream() const
{ return di_.empty() ? false : di_.back() == data_item::stream; }

} // namespace nx
