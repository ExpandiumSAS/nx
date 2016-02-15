#ifndef __NX_FILE_H__
#define __NX_FILE_H__

#include <string>
#include <queue>

#include <nx/config.h>

namespace nx {

/// @file
///
/// sendfile() support
struct NX_API file
{
    std::string path;
    int fd;
    std::size_t offset;
    std::size_t total;

    init();
};

using file_queue = std::queue<file>;

NX_API
bool
send_file(file& f);

} // namespace nx

#endif // __NX_FILE_H__
