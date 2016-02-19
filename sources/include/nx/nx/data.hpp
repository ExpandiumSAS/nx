#ifndef __NX_DATA_H__
#define __NX_DATA_H__

#include <sstream>
#include <vector>
#include <memory>

#include <nx/config.h>
#include <nx/file.hpp>
#include <nx/socket_base.hpp>

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

    void clear();

    template <typename Socket>
    void operator()(Socket& s) const
    {
        auto sit = streams_.begin();
        auto fit = files_.begin();

        for (auto& i : items_) {
            switch (i) {
                case data_item::stream:
                s << (*sit)->str(); ++sit;
                break;
                case data_item::file:
                s << *fit; ++fit;
                break;
            }
        }
    }

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

template <
    typename Socket,
    typename = std::enable_if_t<
        std::is_base_of<socket_base, Socket>::value
    >
>
Socket&
operator<<(Socket& s, const data& d)
{
    d(s);

    return s;
}

} // namespace nx

#endif // __NX_DATA_H__
