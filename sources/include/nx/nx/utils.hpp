#ifndef __NX_UTILS_H__
#define __NX_UTILS_H__

#include <stdlib.h>

#include <string>
#include <sstream>
#include <vector>
#include <regex>
#include <typeinfo>

#include <nx/config.h>

namespace nx {

using strings = std::vector<std::string>;

NX_API
std::string
demangle_type_name(const std::string& mangled);

template <typename T>
inline
std::string
type_info(const T& t)
{ return demangle_type_name(typeid(t).name()); }

template <typename T>
inline
std::string
type_info()
{ return demangle_type_name(typeid(T).name()); }

NX_API
std::string
lc(const std::string& s);

NX_API
void split(
    const std::string& re,
    const std::string& expr,
    strings& list
);

NX_API
bool
match(const std::string& expr, const std::string& re);

NX_API
bool
match(const std::string& expr, const std::string& re, std::smatch& m);

NX_API
std::string
subst(const std::string& s, const std::string& re, const std::string& what);

NX_API
std::string
clean_path(const std::string& path);

inline
strings
split(const std::string& re, const std::string& expr)
{
    strings list;
    split(re, expr, list);

    return std::move(list);
}

template <typename S>
inline
std::string
join(const std::string& sep, const std::vector<S>& v)
{
    std::string joined;

    if (v.size() > 0) {
        joined = v[0];
    }

    for (std::size_t i = 1; i < v.size(); i++) {
        joined += sep;
        joined += v[i];
    }

    return std::move(joined);
}

template <typename T>
struct make_vector {
    typedef std::vector<T> vector_type;

    static
    void make(vector_type& v)
    {}

    template <typename Item, typename... Items>
    static
    void make(vector_type& v, Item& i, Items... items)
    {
        std::ostringstream oss;
        oss << i;
        v.push_back(oss.str());
        make(v, items...);
    }
};

template <typename... Args>
std::string
join(const std::string& sep, Args... args)
{
    std::vector<std::string> v;
    make_vector<std::string>::make(v, args...);

    return join(sep, v);
}

template <typename T>
inline
T to_num(const std::string& s)
{
    return ((T) strtoull(s.c_str(), NULL, 10));
}

} // namespace nx

#endif // __NX_UTILS_H__
