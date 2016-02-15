#ifndef __NX_FILE_H__
#define __NX_FILE_H__

#include <string>
#include <queue>

namespace nx {

/// @file
///
/// sendfile() support
struct file
{
    std::string path;
    int fd;
};

using file_queue = std::queue<file>;

} // namespace nx

#endif // __NX_FILE_H__
