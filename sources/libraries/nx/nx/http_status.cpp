#include <sstream>

#include <nx/http_status.hpp>

namespace nx {

http_status::http_status()
{}

http_status::http_status(code_type c, const char* s)
: code(c),
status(s)
{}

http_status::http_status(
    code_type c,
    const std::string& s,
    const std::string& what
)
: code(c),
status(s),
error(what)
{}

const char*
http_status::what() const noexcept
{
    std::ostringstream oss;

    oss << code << " " << status;

    if (is_error()) {
        oss << ": " << error;
    }

    return oss.str().c_str();
}

bool
http_status::operator==(const http_status& other) const
{ return code == other.code; }

bool
http_status::operator!=(const http_status& other) const
{ return !(*this == other); }

http_status
http_status::operator()(const std::string& what) const
{ return http_status{ code, status, what }; }

http_status
http_status::operator()(
    const std::string& what,
    const std::exception& e
) const
{ return http_status{ code, status, what + ": " + e.what() }; }

http_status
http_status::operator()(const std::exception& e) const
{ return http_status{ code, status, e.what() }; }

bool
http_status::is_error() const
{ return !error.empty(); }

} // namespace nx
