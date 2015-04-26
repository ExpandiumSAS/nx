#ifndef __NX_HTTP_CODE_H__
#define __NX_HTTP_CODE_H__

#include <stdint.h>

#include <nx/config.h>

namespace nx {

class NX_API http_status : public std::exception
{
public:
    using code_type = uint16_t;

    http_status();
    http_status(code_type c, const char* s);
    http_status(
        code_type c,
        const std::string& s,
        const std::string& what
    );

    virtual const char* what() const noexcept;

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
const http_status BadResponse = { 0, "Bad Response" };
const http_status InternalClientError = { 1, "Internal client error" };

// 1xx - Info
const http_status SwitchingProtocols = { 101, "Switching Protocols" };

// 2xx - Success
const http_status OK = { 200, "OK" };
const http_status Created = { 201, "Created" };
const http_status Accepted = { 202, "Accepted" };
const http_status NonAuthoritativeInformation = {
    203, "Non-Authoritative Information"
};
const http_status NoContent = { 204, "No Content" };
const http_status ResetContent = { 205, "Reset Content" };
const http_status PartialContent = { 206, "Partial Content" };

// 4xx - Error
const http_status BadRequest = { 400, "Bad Request" };
const http_status Forbidden = { 403, "Forbidden" };
const http_status NotFound = { 404, "Not Found" };
const http_status MethodNotAllowed = { 405, "Method Not Allowed" };
const http_status InternalServerError = { 500, "Internal server error" };

} // namespace nx

#endif // __NX_HTTP_CODE_H__
