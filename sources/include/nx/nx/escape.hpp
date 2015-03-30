#ifndef __NX_ESCAPE_H__
#define __NX_ESCAPE_H__

#include <string>

#include <nx/config.h>

namespace nx {

NX_API
std::string
escape(const std::string& s);

NX_API
std::string
unescape(const std::string& s);

} // namespace nx

#endif // __NX_ESCAPE_H__
