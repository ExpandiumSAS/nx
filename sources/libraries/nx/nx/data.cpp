#include <cxxu/utils.hpp>

#include <nx/data.hpp>

namespace nx {

std::size_t
data::size() const
{ return size_; }

void
data::clear()
{
    items_.clear();
    streams_.clear();
    files_.clear();
}

data&
data::operator<<(const file& f)
{
    if (cxxu::file_exists(f.path)) {
        make_file(f);
        size_ += cxxu::file_size(f.path);
    }

    return *this;
}

data::stream_type&
data::make_stream()
{
    if (items_.empty() || items_.back() != data_item::stream) {
        items_.emplace_back(data_item::stream);
        auto ptr = std::make_unique<stream_type>();
        streams_.emplace_back(std::move(ptr));
    }

    return *streams_.back();
}

std::size_t
data::stream_size()
{
    std::streamoff o = streams_.back()->tellp();

    return o == -1 ? 0 : o;
}

void
data::make_file(const file& f)
{
    items_.emplace_back(data_item::file);
    files_.emplace_back(f);
}

} // namespace nx
