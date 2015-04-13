#ifndef __NX_HTTP_CODE_H__
#define __NX_HTTP_CODE_H__

#include <stdint.h>

#include <ostream>

#include <nx/config.h>

namespace nx {

struct NX_API http_status
{
    using code_type = uint16_t;

    bool operator==(const http_status& other) const;
    bool operator!=(const http_status& other) const;

    http_status operator()(const std::string& what) const;
    http_status operator()(
        const std::string& what,
        const std::exception& e
    ) const;
    http_status operator()(const std::exception& e) const;

    bool is_error() const;

    code_type code;
    std::string status;
    std::string error;
};

// Internal
const http_status BadResponse = { 0, "Bad Response", "" };
const http_status InternalClientError = { 1, "Internal client error", "" };

// 2xx - Success
const http_status OK = { 200, "OK", "" };
const http_status Created = { 201, "Created", "" };
const http_status Accepted = { 202, "Accepted", "" };
const http_status NonAuthoritativeInformation = {
    203, "Non-Authoritative Information", ""
};
const http_status NoContent = { 204, "No Content", "" };
const http_status ResetContent = { 205, "Reset Content", "" };
const http_status PartialContent = { 206, "Partial Content", "" };

// 4xx - Error
const http_status BadRequest = { 400, "Bad Request", "" };
const http_status Forbidden = { 403, "Forbidden", "" };
const http_status NotFound = { 404, "Not Found", "" };
const http_status MethodNotAllowed = { 405, "Method Not Allowed", "" };
const http_status InternalServerError = { 500, "Internal server error", "" };

inline
std::ostream&
operator<<(std::ostream& os, const http_status& c)
{
    os << c.code << " " << c.status;

    if (c.is_error()) {
        os << ": " << c.error;
    }

    return os;
}

} // namespace nx

#endif // __NX_HTTP_CODE_H__
