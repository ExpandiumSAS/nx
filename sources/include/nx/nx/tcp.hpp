#ifndef __NX_TCP_H__
#define __NX_TCP_H__

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <functional>

#include <nx/handle.hpp>
#include <nx/endpoint.hpp>

namespace nx {

namespace tags {

struct on_connect_tag : callback_tag {};

const on_connect_tag on_connect = {};

} // namespace tags

template <typename Derived, typename... Callbacks>
class tcp_base
: public handle<
    Derived,
    callback<tags::on_connect_tag, Derived&>,
    Callbacks...
>
{
public:
    using base_type = handle<
        Derived,
        callback<tags::on_connect_tag, Derived&>,
        Callbacks...
    >;
    using this_type = tcp_base<Derived, Callbacks...>;
    using accept_cb = std::function<void(Derived&& t)>;
    using connect_cb = std::function<void(Derived& t)>;
    using read_cb = typename callback_signature<
        tags::on_read_tag,
        base_type
    >::type;

    tcp_base()
    : base_type(socket(PF_INET, SOCK_STREAM, 0))
    {}

    tcp_base(int fh, const endpoint& local, const endpoint& remote)
    : base_type(fh),
    local_(local),
    remote_(remote)
    {}

    void connect(const endpoint& to, connect_cb cb)
    {
        remote_ = to;
        local_ = to;
        base_type::handler(tags::on_connect) = cb;

        base_type::handler(tags::on_drain) = [&](Derived& h) {
            // Socket is connected
            uint32_t size = local_.size();
            getsockname(base_type::fh(), local_, &size);
            base_type::handler(tags::on_connect)(base_type::derived());
            base_type::handler(tags::on_connect) = nullptr;
            base_type::start_read();
            base_type::handler(tags::on_drain) = nullptr;
        };

        base_type::start_write();

        int rc = ::connect(base_type::fh(), remote_, remote_.size());

        if (rc != 0 && errno != EINPROGRESS) {
            base_type::handle_error("connect error", errno);
            return;
        }
    }

    const endpoint& serve(const endpoint& from, accept_cb acb, read_cb rcb)
    {
        local_ = from;
        accept_cb_ = acb;
        read_cb_ = rcb;

        bool error = base_type::handle_error(
            "bind error",
            bind(base_type::fh(), local_, local_.size())
        );

        uint32_t size = local_.size();
        base_type::handle_error(
            "getsockname",
            getsockname(base_type::fh(), local_, &size)
        );

        error = error || base_type::handle_error(
            "listen error",
            listen(base_type::fh(), 128)
        );

        if (!error) {
            local_.update();

            base_type::handler(tags::on_read) = [&](Derived& h, buffer& b) {
                // New connection
                endpoint r = local_;
                uint32_t size;

                int fd = accept(base_type::fh(), r, &size);

                if (!base_type::handle_error("accept error", fd)) {
                    Derived client(fd, local_, r);

                    client.handler(tags::on_read) = read_cb_;
                    client.start_read();
                    accept_cb_(std::move(client));
                }
            };
        }

        return local_;
    }

    const endpoint& local() const
    { return local_; }

    const endpoint& remote() const
    { return remote_; }

private:
    endpoint local_;
    endpoint remote_;
    accept_cb accept_cb_;
    read_cb read_cb_;
};

class tcp : public tcp_base<tcp>
{
    using base_type = tcp_base<tcp>;

    using base_type::tcp_base;
};

} // namesapce nx

#endif // __NX_TCP_H__
