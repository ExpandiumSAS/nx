#ifndef __NX_DATA_H__
#define __NX_DATA_H__

#include <sstream>
#include <vector>
#include <memory>

#include <nx/config.h>
#include <nx/file.hpp>

namespace nx {

enum class data_item
{
    stream,
    file
};

using data_items = std::vector<data_item>;

class NX_API data
{
public:
    std::size_t size() const;

    template <typename T>
    data& operator<<(const T& v)
    {
        auto& s = make_stream();
        auto old_size = stream_size();

        s << v;

        size_ += stream_size() - old_size;

        return *this;
    }

    data& operator<<(const file& f);
    data& operator<<(const data& other);

private:
    using stream_type = std::ostringstream;
    using stream_ptr = std::unique_ptr<stream_type>;
    using streams = std::vector<stream_ptr>;
    using files = std::vector<file>;

    stream_type& make_stream();
    std::size_t stream_size();

    void make_file(const file& f);

    std::size_t size_{ 0 };
    data_items items_;
    streams streams_;
    files files_;
};

} // namespace nx

#endif // __NX_DATA_H__
