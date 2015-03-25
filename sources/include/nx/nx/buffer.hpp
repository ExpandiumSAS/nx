#ifndef __NX_BUFFER_H__
#define __NX_BUFFER_H__

#include <cstring>
#include <ostream>
#include <vector>

namespace nx {

/// @file
///
/// Base buffer type and streaming operations

/// Read/write buffer
using buffer = std::vector<char>;

/// Generic output iterator-based streaming support
///
/// @tparam T any type supporting begin() and end()
template <typename T>
inline
buffer&
operator<<(buffer& b, const T& v)
{
    b.insert(b.end(), v.begin(), v.end());
    return b;
}

/// Generic output move-iterator-based streaming support
///
/// @tparam T any type supporting begin() and end()
template <typename T>
inline
buffer&
operator<<(buffer& b, T&& v)
{
    b.insert(
        b.end(),
        std::make_move_iterator(v.begin()),
        std::make_move_iterator(v.end())
    );
    return b;
}

/// Generic intput iterator-based streaming support
///
/// @tparam T any type supporting iterator insert
template <typename T>
inline
buffer&
operator>>(buffer& b, T& v)
{
    v.insert(v.end(), b.begin(), b.end());
    b.clear();
    return b;
}

/// Output for C-style strings
inline
buffer&
operator<<(buffer& b, const char* s)
{
    b.insert(b.end(), s, s + std::strlen(s));
    return b;
}

/// Copy buffer to standard ostrem
inline
std::ostream&
operator<<(std::ostream& os, const buffer& b)
{
    os.write(b.data(), b.size());
    return os;
}

/// Compare buffer content with a string
inline
bool
operator==(const buffer& b, const std::string& s)
{
    if (b.size() != s.size()) {
        return false;
    }

    return std::memcmp(b.data(), s.data(), b.size()) == 0;
}

/// Compare a string with buffer content
inline
bool
operator==(const std::string& s, const buffer& b)
{ return b == s; }

/// Compare buffer content with a C-style string
inline
bool
operator==(const buffer& b, const char* s)
{
    auto slen = std::strlen(s);

    if (b.size() != slen) {
        return false;
    }

    return std::memcmp(b.data(), s, slen) == 0;
}

/// Compare a C-style string with buffer content
inline
bool
operator==(const char* s, const buffer& b)
{ return b == s; }

} // namespace nx

#endif // __NX_BUFFER_H__
