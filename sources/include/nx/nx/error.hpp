#ifndef __NX_ERROR_H__
#define __NX_ERROR_H__

#include <stdint.h>

#include <stdexcept>

#define NX_MAKE_ERROR(NAME, CODE, MSG) \
class NAME : public error \
{ \
public: \
    explicit NAME() \
    : error(CODE, MSG) \
    {} \
    \
    virtual ~NAME() \
    {} \
};

namespace nx {

using code_type = uint16_t;

class error : public std::runtime_error
{
public:
    explicit error(code_type code, const std::string& msg)
    : runtime_error(msg),
    code_(code)
    {}

    virtual ~error()
    {}

    code_type code() const
    { return code_; }

private:
    code_type code_;
};

NX_MAKE_ERROR(bad_response, 0, "Bad Response");
NX_MAKE_ERROR(internal_client_error, 1, "Internal client error");

NX_MAKE_ERROR(bad_request, 400, "Bad Request");
NX_MAKE_ERROR(forbidden, 403, "Forbidden");
NX_MAKE_ERROR(not_found, 404, "Not Found");
NX_MAKE_ERROR(method_not_allowed, 405, "Method Not Allowed");
NX_MAKE_ERROR(internal_server_error, 500, "Internal server error");

} // nx

#undef NX_MAKE_ERROR

#endif // __NX_ERROR_H__
