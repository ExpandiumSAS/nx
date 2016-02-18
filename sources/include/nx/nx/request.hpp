#ifndef __NX_REQUEST_H__
#define __NX_REQUEST_H__

#include <string>
#include <ostream>
#include <sstream>
#include <memory>
#include <type_traits>

#include <nx/picohttpparser.h>

#include <nx/config.h>
#include <nx/http_msg.hpp>
#include <nx/methods.hpp>
#include <nx/attributes.hpp>
#include <nx/uri.hpp>
#include <nx/socket_base.hpp>

namespace nx {

class NX_API request : public http_msg
{
public:
    request();
    request(const nx::method& m);
    request(const std::string& method);
    request(const std::string& method, const std::string& path);
    request(const request& other) = delete;
    request(request&& other);
    virtual ~request();

    request& operator=(request&& other);

    bool parse(buffer& b);

    const std::string& method() const;
    const std::string& path() const;

    std::string& a(const std::string& name);
    const std::string& a(const std::string& name) const;

    bool operator==(const nx::method& m) const;
    bool operator!=(const nx::method& m) const;

    request& operator/(const std::string& path);

    using http_msg::operator<<;

    request& operator<<(const nx::method& m);
    request& operator<<(const attribute& a);
    request& operator<<(const attributes& a);

    bool is_form() const;

private:
    std::string method_;
    std::string path_;
    attributes attrs_;
    std::string empty_;

    const char *raw_method_;
    std::size_t raw_method_len_;
    const char *raw_path_;
    std::size_t raw_path_len_;
};

// template <
//     typename Socket,
//     typename = std::enable_if_t<
//         std::is_base_of<socket_base, Socket>::value
//     >
// >
// Socket&
// operator<<(Socket& s, )


} // namespace nx

#endif // __NX_REQUEST_H__
