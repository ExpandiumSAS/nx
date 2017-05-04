#ifndef __NX_LOCAL_SOCKET_H__
#define __NX_LOCAL_SOCKET_H__

#include <nx/socket.hpp>
#include <nx/socket_template_functions.hpp>

namespace nx{

template <typename Derived, typename... Callbacks>
class local_socket_base
: public socket<
    Derived,
    asio::local::stream_protocol::socket,
    Callbacks...
>
{
public:
    using base_type = socket<
        Derived,
        asio::local::stream_protocol::socket,
        Callbacks...
    >;

    using acceptor_type = asio::local::stream_protocol::acceptor;
    using endpoint_type = asio::local::stream_protocol::endpoint;

    using base_type::base_type;

    local_socket_base() = default;
    local_socket_base(asio::io_service& io)
    : base_type(io)
    {}

    local_socket_base(const local_socket_base& other) = delete;
    local_socket_base(local_socket_base&& other) = default;
    local_socket_base& operator=(const local_socket_base& other) = delete;
    local_socket_base& operator=(local_socket_base&& other) = default;

    auto local() const
    { return base_type::sock().local_endpoint(); }

    std::string local_str() const
    {
        std::ostringstream oss;

        oss << local();

        return oss.str();
    }

    auto remote() const
    { return base_type::sock().remote_endpoint(); }

    std::string remote_str() const
    {
        std::ostringstream oss;

        oss << remote();

        return oss.str();
    }

    acceptor_type& make_acceptor()
    {
        acceptor_ptr_ = std::make_unique<acceptor_type>(
            base_type::io_service()
        );

        return *acceptor_ptr_;
    }

    acceptor_type& acceptor()
    { return *acceptor_ptr_; }

    const acceptor_type& acceptor() const
    { return *acceptor_ptr_; }

    void reuse_addr(acceptor_type &, const endpoint_type& from)
    {
        unlink(from.path().c_str());
    }

private:
    using acceptor_ptr = std::unique_ptr<acceptor_type>;

    acceptor_ptr acceptor_ptr_;
};

class local_socket : public local_socket_base<local_socket>
{
    using base_type = local_socket_base<local_socket>;

    using base_type::local_socket_base;
};

} // nx

#endif // __NX_LOCAL_SOCKET_H__