#ifndef __NX_REGEXP_RFC2396_H__
#define __NX_REGEXP_RFC2396_H__

#include <nx/regexp/push_re.hpp>

namespace nx {
namespace regexp {

NX_RE(digit, "[0-9]")
NX_RE(upalpha, "[A-Z]")
NX_RE(lowalpha, "[a-z]")
NX_RE(alpha, "[a-zA-Z]")
NX_RE(alphanum, "[a-zA-Z0-9]")
NX_RE(hex, "[a-fA-F0-9]")
NX_RE(escaped, "(?:%" + hex + hex + ")")
NX_RE(mark, "[\\-_.!~*'()]")
NX_RE(unreserved, "[a-zA-Z0-9\\-_.!~*'()]")
NX_RE(reserved, "[;/?:@&=+\\$,]")
NX_RE(pchar, "(?:[a-zA-Z0-9\\-_.!~*'():@&=+\\$,]|" + escaped + ")")
NX_RE(uric, "(?:[;/?:@&=+\\$,a-zA-Z0-9\\-_.!~*'()]|" + escaped + ")")
NX_RE(urics, "(?:(?:[;/?:@&=+\\$,a-zA-Z0-9\\-_.!~*'()]+|" + escaped + ")*)")

NX_RE(query, urics)
NX_RE(fragment, urics)
NX_RE(param, "(?:(?:[a-zA-Z0-9\\-_.!~*'():@&=+\\$,]+|" + escaped + ")*)")
NX_RE(segment, "(?:" + param + "(?:;" + param + ")*)")
NX_RE(path_segments, "(?:" + segment + "(?:/" + segment + ")*)")
NX_RE(rel_segment, "(?:(?:[a-zA-Z0-9\\-_.!~*'();@&=+\\$,]*|" + escaped + ")+)" )
NX_RE(abs_path, "(?:/" + path_segments + ")")
NX_RE(rel_path, "(?:" + rel_segment + "(?:" + abs_path + ")?)")
NX_RE(path, "(?:(?:" + abs_path + "|" + rel_path + ")?)")

NX_RE(port, "(?:" + digit + "*)")
NX_RE(IPv4address, "(?:" + digit + "+[.]" + digit + "+[.]" + digit + "+[.]" + digit + "+)")
NX_RE(toplabel, "(?:" + alpha + "[a-zA-Z0-9\\-]*" + alphanum + "|" + alpha + ")")
NX_RE(domainlabel, "(?:(?:" + alphanum + "[a-zA-Z0-9\\-]*)?" + alphanum + ")")
NX_RE(hostname, "(?:(?:" + domainlabel + "[.])*" + toplabel + "[.]?)")
NX_RE(host, "(?:" + hostname + "|" + IPv4address + ")")
NX_RE(hostport, "(?:" + host + "(?::" + port + ")?)")

NX_RE(userinfo, "(?:(?:[a-zA-Z0-9\\-_.!~*'();:&=+\\$,]+|" + escaped + ")*)")
NX_RE(userinfo_no_colon, "(?:(?:[a-zA-Z0-9\\-_.!~*'();&=+\\$,]+|" + escaped + ")*)")
NX_RE(server, "(?:(?:" + userinfo + "@)?" + hostport + ")")

NX_RE(reg_name, "(?:(?:[a-zA-Z0-9\\-_.!~*'()\\$,;:@&=+]*|" + escaped + ")+)")
NX_RE(authority, "(?:" + server + "|" + reg_name + ")")

NX_RE(scheme, "(?:" + alpha + "[a-zA-Z0-9+\\-.]*)")

NX_RE(net_path, "(?://" + authority + abs_path + "?)")
NX_RE(uric_no_slash, "(?:[a-zA-Z0-9\\-_.!~*'();?:@&=+\\$,]|" + escaped + ")")
NX_RE(opaque_part, "(?:" + uric_no_slash + "" + urics + ")")
NX_RE(hier_part, "(?:(?:" + net_path + "|" + abs_path + ")(?:[?]" + query + ")?)")

NX_RE(relativeURI, "(?:(?:" + net_path + "|" + abs_path + "|" + rel_path + ")(?:[?]" + query + ")?)")
NX_RE(absoluteURI, "(?:" + scheme + ":(?:" + hier_part + "|" + opaque_part + "))")
NX_RE(URI_reference, "(?:(?:" + absoluteURI + "|" + relativeURI + ")?(?:#" + fragment + ")?)")

} // namespace regexp
} // namespace nx

#include <nx/regexp/pop_re.hpp>

#endif // __NX_REGEXP_RFC2396_H__
