#ifndef __NX_REGEXP_HTTP_H__
#define __NX_REGEXP_HTTP_H__

#include <nx/regexp/rfc2396.hpp>

#include <nx/regexp/push_re.hpp>

namespace nx {
namespace regexp {

NX_RE(
    http_path,
    "(/((" + path_segments + ")(?:[?](" + query + "))?))"
)
NX_RE(
    http_uri,
    "((http)://(" + host + ")(?::(" + port + "))?"
    "(/((" + path_segments + ")(?:[?](" + query + "))?))?)"
)

} // namespace regexp
} // namespace nx

#include <nx/regexp/pop_re.hpp>

#endif // __NX_REGEXP_HTTP_H__
