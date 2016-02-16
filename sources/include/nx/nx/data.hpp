#ifndef __NX_DATA_H__
#define __NX_DATA_H__

#include <sstream>
#include <vector>

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
    template <typename T>
    data& operator<<(const T& v)
    {


        return *this;
    }

private:
    using stream_items = std::vector<std::ostringstream>;
    using file_items = std::vector<file>;

    template <typename T>
    void write_stream(std::ostringstream& oss, const T& v)
    {}

    bool prev_is_stream() const;

    data_items di_;
    stream_items si_;
    file_items fi_;
};

} // namespace nx

#endif // __NX_DATA_H__
