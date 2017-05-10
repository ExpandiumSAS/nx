#ifndef __NX_TCP_H__
#define __NX_TCP_H__

#include <sstream>

#include <nx/socket.hpp>
#include <nx/socket_template_functions.hpp>

namespace nx {

template <typename Derived, typename... Callbacks>
class tcp_base
: public socket<
    Derived,
    asio::ip::tcp::socket,
    Callbacks...
>
{
public:
    using base_type = socket<
        Derived,
        asio::ip::tcp::socket,
        Callbacks...
    >;

    using acceptor_type = asio::ip::tcp::acceptor;
    using endpoint_type = asio::ip::tcp::endpoint;
    using resolver_type = asio::ip::tcp::resolver;

    using base_type::base_type;

    tcp_base() = default;
    tcp_base(asio::io_service& io)
    : base_type(io)
    {}

    tcp_base(const tcp_base& other) = delete;
    tcp_base(tcp_base&& other) = default;
    tcp_base& operator=(const tcp_base& other) = delete;
    tcp_base& operator=(tcp_base&& other) = default;

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

    resolver_type& make_resolver()
    {
        resolver_ptr_ = std::make_unique<resolver_type>(
            base_type::io_service()
        );

        return *resolver_ptr_;
    }

    resolver_type& resolver()
    { return *resolver_ptr_; }

    const resolver_type& resolver() const
    { return *resolver_ptr_; }

    void reuse_addr(acceptor_type& a, const endpoint_type&)
    {
        a.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    }

private:
    using acceptor_ptr = std::unique_ptr<acceptor_type>;
    using resolver_ptr = std::unique_ptr<resolver_type>;

    acceptor_ptr acceptor_ptr_;
    resolver_ptr resolver_ptr_;
};

class tcp : public tcp_base<tcp>
{
    using base_type = tcp_base<tcp>;

    using base_type::tcp_base;
};

} // namespace nx

#endif // __NX_TCP_H__
