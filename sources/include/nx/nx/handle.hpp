#ifndef __NX_HANDLE_H__
#define __NX_HANDLE_H__

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <iostream>
#include <type_traits>
#include <memory>
#include <queue>

#include <nx/config.h>

#include <nx/callbacks.hpp>
#include <nx/buffer.hpp>
#include <nx/watchers.hpp>
#include <nx/error_code.hpp>

namespace nx {

namespace tags {

struct on_error_tag : callback_tag {};
struct on_read_tag : callback_tag {};
struct on_drain_tag : callback_tag {};
struct on_eof_tag : callback_tag {};

const on_error_tag on_error = {};
const on_read_tag on_read = {};
const on_drain_tag on_drain = {};
const on_eof_tag on_eof = {};

} // namespace tags

template <typename Derived, typename... Callbacks>
class handle
{
public:
    using this_type = handle<Derived, Callbacks...>;
    using callbacks = nx::callbacks<
        callback<tags::on_error_tag, Derived&, const error_code&>,
        callback<tags::on_read_tag, Derived&, buffer&>,
        callback<tags::on_eof_tag, Derived&>,
        callback<tags::on_drain_tag, Derived&>,
        Callbacks...
    >;
    using ptr_type = std::shared_ptr<Derived>;

    handle(int fh) noexcept
    : fh_(fh)
    {
        handle_error(
            "setting non-blocking I/O",
            fcntl(fh_, F_SETFL, fcntl(fh_, F_GETFL, 0) | O_NONBLOCK)
        );

        io_ = [&](int events) {
            if (events & ERROR) {
                handle_error("read/write error", errno);
                return;
            }

            if (events & READ) {
                handle_read();
            }

            if (events & WRITE) {
                handle_write();
            }
        };
    }

    handle(const this_type& other) = delete;

    virtual ~handle()
    {}

    int fh() const
    { return fh_; }

    template <
        typename Tag,
        typename Enabled = typename std::enable_if<
            std::is_base_of<callback_tag, Tag>::value
        >::type
    >
    auto&
    handler(const Tag& t)
    { return callbacks_.get(t); }

    template <typename Tag>
    auto&
    operator[](const Tag& t)
    { return handler(t); }

    this_type&
    operator<<(const char* s)
    {
        wq_.emplace(s, s + std::strlen(s));
        start_write();

        return *this;
    }

    this_type&
    operator<<(const std::string& s)
    {
        wq_.emplace(s.begin(), s.end());
        start_write();

        return *this;
    }

    this_type&
    operator<<(std::string&& s)
    {
        wq_.emplace(
            std::make_move_iterator(s.begin()),
            std::make_move_iterator(s.end())
        );
        start_write();

        return *this;
    }

    this_type&
    operator<<(buffer&& b)
    {
        wq_.emplace(std::move(b));
        start_write();

        return *this;
    }

protected:
    bool handle_error(const error_code& e)
    {
        if (!e) {
            return false;
        }

        if (!handler(tags::on_error)(derived(), e)) {
            // Unhandled error
            std::cerr << "unhandled error: " << e << std::endl;
        }

        return true;
    }

    bool handle_error(const char* what, int status)
    { return handle_error(error_code(what, status)); }

    void start_read()
    {
        io_ |= READ;
        io_.start();
    }

    void start_write()
    {
        io_ |= WRITE;
        io_.start();
    }

    /// CRTP interface
    Derived* derived_this()
    { return static_cast<Derived*>(this); }

    Derived& derived()
    { return *static_cast<Derived* const>(this); }
    Derived const& derived() const
    { return *static_cast<Derived const*>(this); }

private:
    void handle_read()
    {
        int available;

        handle_error(
            "error reading available bytes",
            ioctl(fh_, FIONREAD, &available)
        );

        std::size_t cur_size = rbuf_.size();
        rbuf_.resize(cur_size + (std::size_t) available);

        void* buf = &(rbuf_.data()[cur_size]);

        ssize_t nread = read(fh_, buf, (std::size_t) available);

        if (nread < 0) {
            handle_error("read error", nread);
            return;
        }

        if (nread == 0) {
            io_.stop();
            handler(tags::on_eof)(derived());
        } else {
            handler(tags::on_read)(derived(), rbuf_);
        }
    }

    void handle_write()
    {
        if (wq_.empty()) {
            io_ ^= WRITE;
            handler(tags::on_drain)(derived());
            return;
        }

        buffer& b = wq_.front();
        auto buf = static_cast<const void*>(b.data());
        std::size_t size = b.size();

        ssize_t nwritten = write(fh_, buf, size);

        if (nwritten < 0) {
            if (errno != EAGAIN && errno != EINTR && errno != EWOULDBLOCK) {
                handle_error("write error", errno);
            }

            return;
        }

        std::size_t wsize = nwritten;

        if (wsize == size) {
            wq_.pop();
        } else {
            b.erase(b.begin(), b.begin() + wsize);
        }
    }

private:
    using queue_type = std::queue<buffer>;

    int fh_;
    callbacks callbacks_;
    queue_type wq_;
    buffer rbuf_;
    io io_;
    std::size_t read_size_ = 1024 * 1024;
};

} // namespace nx

#endif // __NX_HANDLE_H__
