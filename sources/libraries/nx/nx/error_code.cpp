#include <errno.h>

#include <nx/error_code.hpp>

namespace nx {

error_code
make_error_code()
{
    return error_code(
        errno,
        boost::system::system_category()
    );
}

} // namespace nx
