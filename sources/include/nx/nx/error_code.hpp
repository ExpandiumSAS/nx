#ifndef __NX_ERROR_CODE_H__
#define __NX_ERROR_CODE_H__

#include <boost/system/error_code.hpp>

#include <nx/config.h>

namespace nx {

using error_code = boost::system::error_code;

NX_API
error_code
make_error_code();

} // namespace nx

#endif // __NX_ERROR_CODE_H__
