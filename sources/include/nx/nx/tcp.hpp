#ifndef __NX_TCP_H__
#define __NX_TCP_H__

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <functional>

#include <nx/handle.hpp>
#include <nx/endpoint.hpp>
#include <nx/callback_access.hpp>

namespace nx {

template <typename Derived, typename... Callbacks>
class tcp_base
: public handle<
    Derived,
    Callbacks...
>
{
public:
    using base_type = handle<
        Derived,
        Callbacks...
    >;
    using this_type = tcp_base<Derived, Callbacks...>;

    tcp_base()
    : base_type(socket(PF_INET, SOCK_STREAM, 0))
    {}

    tcp_base(int fh)
    : base_type(fh)
    { update_endpoints(); }

    tcp_base(this_type&& other)
    : base_type(std::forward<base_type>(other)),
    local_(std::move(other.local_)),
    remote_(std::move(other.remote_))
    {}

    virtual ~tcp_base()
    {}

    tcp_base(const this_type& other) = delete;
    this_type& operator=(const this_type& other) = delete;

    this_type& operator=(this_type&& other)
    {
        base_type::operator=(std::forward<base_type>(other));
        local_ = std::move(other.local_);
        remote_ = std::move(other.remote_);

        return *this;
    }

    endpoint& local()
    { return local_; }

    const endpoint& local() const
    { return local_; }

    endpoint& remote()
    { return remote_; }

    const endpoint& remote() const
    { return remote_; }

    void update_endpoints()
    {
        local_.set_from_local(base_type::fh());
        remote_.set_from_remote(base_type::fh());
    }

private:
    endpoint local_;
    endpoint remote_;
};

class tcp : public tcp_base<tcp>
{
    using base_type = tcp_base<tcp>;

    using base_type::tcp_base;
};

template <typename Handle, typename Connected>
Handle&
connect(
    Handle& h,
    const endpoint& to,
    Connected&& cb
)
{
    h.set_nonblocking();
    h.remote() = to;

    h[tags::on_drain] = [cb = std::move(cb)](Handle& h) {
        // Socket is writable (connected)
        h.update_endpoints();
        cb(h);
        h.start_read();
        h[tags::on_drain] = nullptr;
    };

    h.start_write();

    int rc = ::connect(h.fh(), h.remote(), h.remote().size());

    if (rc != 0 && errno != EINPROGRESS) {
        handle_error(h, "connect error", errno);
    }

    return h;
}

template <typename Handle, typename Connected>
Handle&
connect(
    const endpoint& to,
    Connected&& cb
)
{
    auto p = new_handle<Handle>();
    auto& h = *p;

    return connect(h, to, std::move(cb));
}

template <typename Handle, typename Accepted, typename Readable>
const endpoint&
serve(
    Handle& h,
    const endpoint& from,
    Accepted&& accept_cb,
    Readable&& read_cb
)
{
    h.local() = from;

    bool error = handle_error(
        h,
        "bind error",
        bind(h.fh(), h.local(), h.local().size())
    );

    h.local().set_from_local(h.fh());

    error = error || handle_error(
        h,
        "listen error",
        listen(h.fh(), 128)
    );

    if (!error) {
        h.set_nonblocking();

        h[tags::on_readable] = [
            accept_cb = std::move(accept_cb),
            read_cb = std::move(read_cb)
        ](Handle& h) {
            // New connection
            endpoint remote;
            uint32_t size = 0;

            int fd = ::accept(h.fh(), remote, &size);

            if (fd == -1) {
                handle_error(h, "accept error", fd);
            } else {
                auto cp = new_handle<Handle>(fd);
                auto& c = *cp;

                c[tags::on_stopped] = [](Handle& c) {
                    handle_error(
                        c,
                        "shutdown",
                        shutdown(c.fh(), SHUT_WR)
                    );
                };
                c[tags::on_read] = std::move(read_cb);
                c.start_read();
                accept_cb(c);
            }
        };

        h.start_read(true);
    }

    return h.local();
}

template <typename Handle, typename Accepted, typename Readable>
const endpoint&
serve(
    const endpoint& from,
    Accepted&& accept_cb,
    Readable&& read_cb
)
{
    auto p = new_handle<Handle>();
    auto& h = *p;

    return serve(h, from, std::move(accept_cb), std::move(read_cb));
}

} // namesapce nx

#endif // __NX_TCP_H__
