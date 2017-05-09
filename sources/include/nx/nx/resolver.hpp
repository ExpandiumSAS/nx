#pragma once

#include <boost/asio.hpp>
#include <nx/endpoint.hpp>
#include <nx/service.hpp>

namespace nx {

inline 
endpoint_tcp
resolve_endpoint(const std::string& host, uint16_t port = 0)
{
    auto t = nx::service::get().available_task();
    boost::system::error_code ec;
    endpoint_tcp result;

    asio::ip::tcp::resolver::query q(host, std::to_string(port), asio::ip::resolver_query_base::address_configured);
    asio::ip::tcp::resolver resolver(t->get_io_service());
    asio::ip::tcp::resolver::iterator end;
    auto it = resolver.resolve(q, ec);

    if (it == end || ec) {
        throw std::runtime_error("nx::resolve_endpoint end point not found");
    }  
    result = it->endpoint();
    nx::service::get().remove_task(t);

    return result;
}   

}