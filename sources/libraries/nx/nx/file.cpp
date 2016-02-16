#if defined(LINUX)
#include <sys/sendfile.h>
#elif defined(DARWIN)
#include <sys/socket.h>
#include <sys/uio.h>
#endif

#include <nx/file.hpp>

namespace nx {

ssize_t
send_file(int out_fd, int in_fd, off_t* offset, size_t count)
{
#if defined(LINUX)
    return ::sendfile(out_fd, in_fd, offset, count);
#elif defined(DARWIN)
    off_t bytes = count;
    auto ret = ::sendfile(in_fd, out_fd, *offset, &bytes, NULL, 0);

    if (ret == -1) {
        return -1;
    }

    *offset += bytes;
    return bytes;
#endif
}

} // namespace nx
