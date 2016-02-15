#include <fcntl.h>

#if defined(LINUX)
#include <sys/sendfile.h>
#elif defined(DARWIN)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#endif

#include <cxxu/utils.hpp>

#include <nx/file.hpp>

namespace nx {

file::init()
{
    fd = 0;
    offset = 0;
    total = 0;

    if (cxxu::file_exists(path)) {
        total = cxxu::file_size(path);
    }

    fd = ::open(path.c_str(), O_RDONLY);

    if (f.fd == -1) {
        auto ec = error_code(
            errno,
            boost::system::system_category()
        );

        if (handle_error(derived(), "sendfile open", ec)) {
            return;
        }
    } else {

    }
}

bool
send_file(file& f)
{

}

} // namespace nx

