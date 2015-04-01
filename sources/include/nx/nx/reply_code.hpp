#ifndef __NX_REPLY_CODE_H__
#define __NX_REPLY_CODE_H__

#include <stdint.h>

#include <ostream>

#include <nx/config.h>

namespace nx {

struct reply_code
{
    using code_type = uint16_t;

    bool operator==(const reply_code& other) const
    { return code == other.code; }

    bool operator!=(const reply_code& other) const
    { return !(*this == other); }

    code_type code;
    std::string status;
};

const reply_code OK = { 200, "OK" };
const reply_code Created = { 201, "Created" };
const reply_code Accepted = { 202, "Accepted" };
const reply_code NonAuthoritativeInformation = {
    203, "Non-Authoritative Information"
};
const reply_code NoContent = { 204, "No Content" };
const reply_code ResetContent = { 205, "Reset Content" };
const reply_code PartialContent = { 206, "Partial Content" };

inline
std::ostream&
operator<<(std::ostream& os, const reply_code& c)
{
    os << c.code << " " << c.status;

    return os;
}

} // namespace nx

#endif // __NX_REPLY_CODE_H__
