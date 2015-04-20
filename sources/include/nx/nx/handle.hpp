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
#include <atomic>

#include <nx/config.h>

#include <nx/handle_base.hpp>
#include <nx/callbacks.hpp>
#include <nx/buffer.hpp>
#include <nx/watchers.hpp>
#include <nx/error_code.hpp>

namespace nx {

namespace tags {

struct on_error_tag : callback_tag {};
struct on_read_tag : callback_tag {};
struct on_readable_tag : callback_tag {};
struct on_writable_tag : callback_tag {};
struct on_drain_tag : callback_tag {};
struct on_eof_tag : callback_tag {};
struct on_close_tag : callback_tag {};

const on_error_tag on_error = {};
const on_read_tag on_read = {};
const on_readable_tag on_readable = {};
const on_writable_tag on_writable = {};
const on_drain_tag on_drain = {};
const on_eof_tag on_eof = {};
const on_close_tag on_close = {};

} // namespace tags

template <typename Handle>
bool
handle_error(Handle& h, const error_code& e)
{
    if (!e) {
        return false;
    }

    if (!h.handler(tags::on_error)(h, e)) {
        // Unhandled error
        std::cerr << "unhandled error: " << e << std::endl;
    }

    return true;
}

template <typename Handle>
bool
handle_error(Handle& h, const char* what, int status)
{ return handle_error(h, error_code(what, status)); }

template <typename Derived, typename... Callbacks>
class handle : public handle_base
{
public:
    using this_type = handle<Derived, Callbacks...>;
    using callbacks = nx::callbacks<
        callback<tags::on_error_tag, Derived&, const error_code&>,
        callback<tags::on_read_tag, Derived&>,
        callback<tags::on_readable_tag, Derived&>,
        callback<tags::on_writable_tag, Derived&>,
        callback<tags::on_drain_tag, Derived&>,
        callback<tags::on_eof_tag, Derived&>,
        callback<tags::on_close_tag, Derived&>,
        Callbacks...
    >;

    handle(int fh) noexcept
    : fh_(fh),
    io_(fh)
    { set_io_cb(); }

    handle(this_type&& other) noexcept
    : fh_(other.fh_),
    io_(std::move(other.io_)),
    read_notify_only_(other.read_notify_only_.load()),
    write_notify_only_(other.write_notify_only_.load()),
    closing_(other.closing_.load()),
    closed_(other.closed_.load()),
    eof_(other.eof_.load()),
    callbacks_(std::move(other.callbacks_)),
    wq_(std::move(other.wq_)),
    rbuf_(std::move(other.rbuf_)),
    read_size_(other.read_size_)
    {
        set_io_cb();
        other.fh_ = -1;
        other.closed_ = true;
        other.eof_ = true;
    }

    handle(const this_type& other) = delete;
    this_type& operator=(const this_type& other) = delete;

    virtual ~handle()
    {}

    this_type& operator=(this_type&& other) noexcept
    {
        fh_ = other.fh_;
        io_ = std::move(other.io_);
        set_io_cb();
        read_notify_only_ = other.read_notify_only_.load();
        write_notify_only_ = other.write_notify_only_.load();
        closing_ = other.closing_.load();
        closed_ = other.closed_.load();
        eof_ = other.eof_.load();
        callbacks_ = std::move(other.callbacks_);
        wq_ = std::move(other.wq_);
        rbuf_ = std::move(other.rbuf_);
        read_size_ = other.read_size_;

        other.fh_ = -1;
        other.closed_ = true;
        other.eof_ = true;

        return *this;
    }

    void push_close()
    {
        if (closing_) {
            return;
        }

        closing_ = true;

        io_ ^= NX_READ;

        start_write();
    }

    void set_nonblocking()
    {
        handle_error(
            derived(),
            "setting non-blocking I/O",
            fcntl(fh_, F_SETFL, fcntl(fh_, F_GETFL, 0) | O_NONBLOCK)
        );
    }

    int fh() const
    { return fh_; }

    buffer& rbuf()
    { return rbuf_; }

    const buffer& rbuf() const
    { return rbuf_; }

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
    { return push_write(s, s + std::strlen(s)); }

    this_type&
    operator<<(const std::string& s)
    { return push_write(s.begin(), s.end()); }

    this_type&
    operator<<(std::string&& s)
    {
        return
            push_write(
                std::make_move_iterator(s.begin()),
                std::make_move_iterator(s.end())
            );
    }

    this_type&
    operator>>(std::string& s)
    {
        s.insert(s.end(), rbuf_.begin(), rbuf_.end());
        rbuf_.clear();

        return *this;
    }

    this_type&
    operator<<(buffer&& b)
    { return push_write(std::move(b)); }

    this_type&
    operator>>(buffer& b)
    {
        b.insert(b.end(), rbuf_.begin(), rbuf_.end());
        rbuf_.clear();

        return *this;
    }

    void read_notify_only(bool flag)
    { read_notify_only_ = flag; }

    void start_read()
    {
        io_ |= NX_READ;
        io_.start();
    }

    void write_notify_only(bool flag)
    { write_notify_only_ = flag; }

    void start_write()
    {
        if (wq_.empty() && closing_) {
            close();
        } else {
            io_ |= NX_WRITE;
            io_.start();
        }
    }

protected:
    template <typename Iterator>
    this_type& push_write(Iterator b, Iterator e)
    {
        if (!closing_) {
            wq_.emplace(b, e);
            start_write();
        }

        return *this;
    }

    template <typename T>
    this_type& push_write(T&& t)
    {
        if (!closing_) {
            wq_.emplace(std::move(t));
            start_write();
        }

        return *this;
    }

    /// CRTP interface
    Derived* derived_this()
    { return static_cast<Derived*>(this); }

    Derived& derived()
    { return *static_cast<Derived* const>(this); }
    Derived const& derived() const
    { return *static_cast<Derived const*>(this); }

private:
    void set_io_cb()
    {
        io_ = [&](int events) {
            std::cout
                << "H: " << fh_ << " " << std::hex << events
                << std::endl;

            if (events & NX_ERROR) {
                handle_error(derived(), "read/write error", errno);
                return;
            }

            if (events & NX_READ) {
                handle_read();
            }

            if (events & NX_WRITE) {
                handle_write();
            }
        };
    }

    void handle_read()
    {
        std::cout
                << "H: " << fh_ << " handle_read"
                << std::endl;

        if (read_notify_only_) {
            std::cout
                << "H: " << fh_ << " read_notify_only"
                << std::endl;

            handler(tags::on_readable)(derived());

            return;
        }

        int available = 0;

        handle_error(
            derived(),
            "error reading available bytes",
            ioctl(fh_, FIONREAD, &available)
        );

        ssize_t nread = 0;

        if (available > 0) {
            std::size_t cur_size = rbuf_.size();
            rbuf_.resize(cur_size + (std::size_t) available);

            void* buf = &(rbuf_.data()[cur_size]);

            nread = read(fh_, buf, (std::size_t) available);
        }

        if (nread < 0) {
            handle_error(derived(), "read error", nread);
            return;
        }

        if (nread == 0) {
            io_.stop([&]() { eof(); });
        } else {
            handler(tags::on_read)(derived());
        }
    }

    void handle_write()
    {
        std::cout
                << "H: " << fh_ << " handle_write"
                << std::endl;

        if (write_notify_only_) {
            std::cout
                << "H: " << fh_ << " write_notify_only"
                << std::endl;

            handler(tags::on_writable)(derived());

            return;
        }

        if (wq_.empty()) {
            if (closing_) {
                close();
            } else {
                io_ ^= NX_WRITE;
                handler(tags::on_drain)(derived());
            }

            return;
        }

        buffer& b = wq_.front();
        auto buf = static_cast<const void*>(b.data());
        std::size_t size = b.size();

        ssize_t nwritten = write(fh_, buf, size);

        if (nwritten < 0) {
            if (errno != EAGAIN && errno != EINTR && errno != EWOULDBLOCK) {
                handle_error(derived(), "write error", errno);
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

    void close()
    {
        if (closed_) {
            return;
        }

        std::cout
                << "H: " << fh_ << " closing"
                << std::endl;

        closed_ = true;

        io_.stop(
            [&]() {
                eof();
                handler(tags::on_close)(derived());
                ::close(fh_);
                unregister_handle(ptr());
            }
        );
    }

    void eof()
    {
        if (eof_) {
            return;
        }

        eof_ = true;

        handler(tags::on_eof)(derived());
    }

private:
    using queue_type = std::queue<buffer>;

    int fh_;
    io io_;
    std::atomic_bool read_notify_only_{false};
    std::atomic_bool write_notify_only_{false};
    std::atomic_bool closing_{false};
    std::atomic_bool closed_{false};
    std::atomic_bool eof_{false};
    callbacks callbacks_;
    queue_type wq_;
    buffer rbuf_;
    std::size_t read_size_ = 1024 * 1024;
};

} // namespace nx

#endif // __NX_HANDLE_H__
