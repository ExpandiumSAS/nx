#include <arpa/inet.h>

#include <cstring>
#include <sstream>

#include <nx/endpoint.hpp>

namespace nx {

bool
to_ip4_addr(const std::string& ip, uint16_t port, ip4_addr* addr)
{
    std::memset((void*) addr, 0, sizeof(ip4_addr));

    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);

    return inet_pton(AF_INET, ip.c_str(), &(addr->sin_addr.s_addr)) == 0;
}

bool
to_ip4_name(const ip4_addr* addr, std::string& ip)
{
    char dest[16];

    bool ok = inet_ntop(AF_INET, &addr->sin_addr, dest, 16) == 0;

    ip = dest;

    return ok;
}

bool
to_ip6_addr(const std::string& ip, uint16_t port, ip6_addr* addr)
{
    std::memset((void*) addr, 0, sizeof(ip6_addr));

    addr->sin6_family = AF_INET6;
    addr->sin6_port = htons(port);

    return inet_pton(AF_INET6, ip.c_str(), &addr->sin6_addr) == 0;
}

bool
to_ip6_name(const ip6_addr* addr, std::string& ip)
{
    char dest[46];

    bool ok = inet_ntop(AF_INET6, &addr->sin6_addr, dest, 46) == 0;

    ip = dest;

    return ok;
}

endpoint::endpoint(const std::string& ip, uint16_t port)
: ip_(ip),
port_(port)
{ init(); }

endpoint::endpoint()
: endpoint("0.0.0.0", 0)
{}

endpoint::endpoint(const ip_addr& addr)
{ init(addr); }

endpoint::endpoint(const ip4_addr& addr)
{ init(addr); }

endpoint::endpoint(const ip6_addr& addr)
{ init(addr); }

endpoint::endpoint(const endpoint& other)
: ip_(other.ip_),
port_(other.port_),
addr_(other.addr_)
{ init(); }

endpoint::endpoint(endpoint&& other)
: ip_(std::move(other.ip_)),
port_(other.port_),
addr_(other.addr_)
{ init(); }

void
endpoint::init()
{
    std::memset((void*) &addr_, 0, sizeof(addr_));

    if (to_ip4_addr(ip_.c_str(), port_, ip4_addr_cast(&addr_)) != 0) {
        to_ip6_addr(ip_.c_str(), port_, ip6_addr_cast(&addr_));
    }
}

void
endpoint::init(const ip_addr& addr)
{
    addr_ = addr;

    if (addr_.sa_family == AF_INET) {
        init(ip4_addr_cast(addr_));
    } else if (addr_.sa_family == AF_INET6) {
        init(ip6_addr_cast(addr_));
    }
}

void
endpoint::init(const ip4_addr& addr)
{
    if (to_ip4_name(&addr, ip_) == 0) {
        port_ = ntohs(addr.sin_port);
    }
}

void
endpoint::init(const ip6_addr& addr)
{
    if (to_ip6_name(&addr, ip_)) {
        port_ = ntohs(addr.sin6_port);
    }
}

endpoint&
endpoint::operator=(const ip_addr& addr)
{
    init(addr);

    return *this;
}

endpoint&
endpoint::operator=(const ip4_addr& addr)
{
    init(addr);

    return *this;
}

endpoint&
endpoint::operator=(const ip6_addr& addr)
{
    init(addr);

    return *this;
}


endpoint&
endpoint::operator=(const endpoint& other)
{
    ip_ = other.ip_;
    port_ = other.port_;
    addr_ = other.addr_;

    return *this;
}

endpoint&
endpoint::operator=(endpoint&& other)
{
    ip_ = std::move(other.ip_);
    port_ = other.port_;
    addr_ = other.addr_;

    return *this;
}

const std::string&
endpoint::ip() const
{ return ip_; }

const char*
endpoint::ip_c_str() const
{ return ip_.c_str(); }

uint16_t
endpoint::port() const
{ return port_; }

std::string
endpoint::str() const
{
    std::ostringstream oss;

    oss << ip_ << ":" << port_;

    return oss.str();
}

endpoint::operator ip_addr_ptr() const
{ return (ip_addr_ptr) &addr_; }

endpoint::operator std::string() const
{ return str(); }

} // namespace nx
