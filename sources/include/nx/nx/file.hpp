#ifndef __NX_FILE_H__
#define __NX_FILE_H__

#include <fcntl.h>

#if defined(LINUX)
#include <sys/sendfile.h>
#elif defined(DARWIN)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#endif

#include <string>
#include <queue>

#include <cxxu/utils.hpp>

#include <nx/config.h>
#include <nx/error_code.hpp>

namespace nx {

/// @file
///
/// sendfile() support
struct file
{
    std::string path;
};

using file_queue = std::queue<file>;

namespace detail {

struct file_state
{
    file f;
    int fd;
    off_t offset;
    off_t total;
};

template <typename Socket, typename Callable>
void send_file_write(Socket& s, file_state fs, Callable cb)
{
    if (fs.offset == fs.total) {
        // We're done
        cb(error_code(), fs.total);
        return;
    }

    auto result = sendfile(
        s.sock().native(),
        fs.fd,
        &fs.offset,
        fs.total - fs.offset
    );

    if (result == -1) {
        auto ec = make_error_code();

        if (ec == asio::error::would_block) {
            asio::async_write(
                s.sock(),
                asio::null_buffers(),
                [&s,fs,cb](const error_code& ec, std::size_t count) {
                    // Socket is writeable
                    send_file_write(s, fs, cb);
                }
            );
        } else {
            cb(ec, fs.offset);
        }
    } else {
        asio::async_write(
            s.sock(),
            asio::null_buffers(),
            [&s,fs,cb](const error_code& ec, std::size_t count) {
                // Socket is writeable
                send_file_write(s, fs, cb);
            }
        );
    }
}

};

template <typename Socket, typename Callable>
void
send_file(Socket& s, const file& f, Callable cb)
{
    auto fs = detail::file_state{ f, -1, 0, 0 };

    if (cxxu::file_exists(fs.f.path)) {
        fs.total = cxxu::file_size(fs.f.path);
    }

    fs.fd = ::open(fs.f.path.c_str(), O_RDONLY);

    if (fs.fd == -1) {
        auto ec = make_error_code();

        s.postpone() << [&s,fs,cb,ec]() {
            cb(ec, fs.total);
        };
    } else {
        detail::send_file_write(s, fs, cb);
    }
}

} // namespace nx

#endif // __NX_FILE_H__
