#ifndef __NX_TCP_H__
#define __NX_TCP_H__

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <tuple>

#include <nx/handle.hpp>
#include <nx/endpoint.hpp>
#include <nx/callback_access.hpp>

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
    using accept_cb = std::function<void(Derived& t)>;
    using connect_cb = std::function<void(Derived& t)>;
    using read_cb = typename callback_signature<
        tags::on_read_tag,
        base_type
    >::type;

    struct connected_tag {};
    struct connection_tag {};

    tcp_base()
    : base_type(socket(PF_INET, SOCK_STREAM, 0))
    {}

    tcp_base(int fh, const endpoint& local, const endpoint& remote)
    : base_type(fh),
    local_(local),
    remote_(remote)
    {}

    tcp_base(this_type&& other)
    { *this = std::move(other); }

    virtual ~tcp_base()
    {}

    tcp_base(const this_type& other) = delete;
    this_type& operator=(const this_type& other) = delete;

    this_type& operator=(this_type&& other)
    {
        base_type::operator=(std::forward<base_type>(other));
        local_ = std::move(other.local_);
        remote_ = std::move(other.remote_);
        accept_cb_ = std::move(other.accept_cb_);
        read_cb_ = std::move(other.read_cb_);
        clients_ = std::move(other.clients_);

        return *this;
    }

    void connect(const endpoint& to, connect_cb cb)
    {
        base_type::set_nonblocking();
        remote_ = to;
        local_ = to;
        base_type::handler(tags::on_connect) = cb;

        base_type::handler(tags::on_drain) = [](Derived& h) {
            // Socket is connected
            callback_access::call<connected_tag>(h);
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

        local_.set_from_local(base_type::fh());

        error = error || base_type::handle_error(
            "listen error",
            listen(base_type::fh(), 128)
        );

        if (!error) {
            base_type::set_nonblocking();

            base_type::handler(tags::on_readable) = [](Derived& h) {
                // New connection
                callback_access::call<connection_tag>(h);
            };

            base_type::start_read(true);
        }

        return local_;
    }

    const endpoint& local() const
    { return local_; }

    const endpoint& remote() const
    { return remote_; }

private:
    friend callback_access;

    void operator()(const connected_tag& t)
    {
        local_.set_from_local(base_type::fh());
        remote_.set_from_remote(base_type::fh());
        base_type::handler(tags::on_connect)(base_type::derived());
        base_type::handler(tags::on_connect) = nullptr;
        base_type::start_read();
        base_type::handler(tags::on_drain) = nullptr;
    }

    void operator()(const connection_tag& t)
    {
        endpoint r = local_;
        uint32_t size = 0;

        int fd = accept(base_type::fh(), r, &size);

        if (fd == -1) {
            base_type::handle_error("accept error", fd);
        } else {
            auto p = clients_.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(r.str()),
                std::forward_as_tuple(
                    std::make_unique<Derived>(fd, local_, r)
                )
            );

            auto& client = p.first->second;

            client->handler(tags::on_read) = read_cb_;
            client->start_read();
            accept_cb_(*client);
        }
    }

private:
    using client_ptr = std::unique_ptr<Derived>;
    using client_map = std::unordered_map<std::string, client_ptr>;

    endpoint local_;
    endpoint remote_;
    accept_cb accept_cb_;
    read_cb read_cb_;
    client_map clients_;
};

class tcp : public tcp_base<tcp>
{
    using base_type = tcp_base<tcp>;

    using base_type::tcp_base;
};

} // namesapce nx

#endif // __NX_TCP_H__
