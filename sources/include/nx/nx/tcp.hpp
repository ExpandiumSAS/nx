#ifndef __NX_TCP_H__
#define __NX_TCP_H__

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <memory>
#include <functional>

#include <boost/asio.hpp>

#include <nx/callback_access.hpp>

namespace nx {

using namespace asio = boost::asio;

template <typename Derived, typename... Callbacks>
class tcp
: public std::enable_shared_from_this<tcp<Derived, Callbacks...>>
{
public:
    using this_type = tcp<Derived, Callbacks...>;

    tcp(const tcp& other) = delete;
    tcp& operator=(const tcp& other) = delete;

    explicit tcp(const std::string& address, const std::string& port)
    {}

    virtual ~tcp_base()
    {}


    this_type& operator=(this_type&& other)
    {
        base_type::operator=(std::forward<base_type>(other));
        local_ = std::move(other.local_);
        remote_ = std::move(other.remote_);

        return *this;
    }

    void set_reuseaddr()
    {
        int yes = 1;

        handle_error(
            base_type::derived(),
            "setting address reuse",
            setsockopt(
                base_type::fh(),
                SOL_SOCKET, SO_REUSEADDR,
                &yes, sizeof(int)
            )
        );
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

    h[tags::on_writable] = [cb = std::move(cb)](Handle& h) {
        // Socket is writable (connected)
        h.write_notify_only(false);
        h.update_endpoints();
        cb(h);
        h.start_read();
        h[tags::on_writable] = nullptr;
    };

    h.write_notify_only(true);
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
    h.set_reuseaddr();

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

                c[tags::on_close] = [](Handle& c) {
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

        h.read_notify_only(true);
        h.start_read();
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
