#include <nx/http_status.hpp>

namespace nx {

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
