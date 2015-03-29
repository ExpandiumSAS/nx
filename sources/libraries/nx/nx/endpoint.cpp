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

    return inet_pton(AF_INET, ip.c_str(), &(addr->sin_addr.s_addr)) == 1;
}

bool
to_ip4_name(const ip4_addr* addr, std::string& ip)
{
    char dest[16];

    bool ok = inet_ntop(AF_INET, &addr->sin_addr, dest, 16) != nullptr;

    ip = dest;

    return ok;
}

bool
to_ip6_addr(const std::string& ip, uint16_t port, ip6_addr* addr)
{
    std::memset((void*) addr, 0, sizeof(ip6_addr));

    addr->sin6_family = AF_INET6;
    addr->sin6_port = htons(port);

    return inet_pton(AF_INET6, ip.c_str(), &addr->sin6_addr) == 1;
}

bool
to_ip6_name(const ip6_addr* addr, std::string& ip)
{
    char dest[46];

    bool ok = inet_ntop(AF_INET6, &addr->sin6_addr, dest, 46) != nullptr;

    ip = dest;

    return ok;
}

void
to_ip_name(const ip_addr* addr, std::string& ip)
{
    if (addr->sa_family == AF_INET) {
        to_ip4_name(ip4_addr_cast(addr), ip);
    } else if (addr->sa_family == AF_INET6) {
        to_ip6_name(ip6_addr_cast(addr), ip);
    }
}

endpoint::endpoint()
: endpoint("0.0.0.0", 0)
{}

endpoint::endpoint(const char* ip, uint16_t port)
{
    std::memset((void*) &addr_, 0, sizeof(addr_));

    if (!to_ip4_addr(ip, port, ip4_addr_cast(&addr_))) {
        if (!to_ip6_addr(ip, port, ip6_addr_cast(&addr_))) {
            std::ostringstream oss;

            oss
                << "failed to initialize endpoint from: '"
                << ip << ", " << port << "'"
                ;

            throw std::runtime_error(oss.str());
        }
    }
}

endpoint::endpoint(const std::string& ip, uint16_t port)
: endpoint(ip.c_str(), port)
{}

endpoint::endpoint(const ip_addr& addr)
{ *this = addr; }

endpoint::endpoint(const ip4_addr& addr)
{ *this = addr; }

endpoint::endpoint(const ip6_addr& addr)
{ *this = addr; }

endpoint::endpoint(const endpoint& other)
{ *this = other.addr_; }

endpoint::endpoint(endpoint&& other)
{ *this = other.addr_; }

endpoint&
endpoint::operator=(const ip_addr& addr)
{
    addr_ = addr;

    return *this;
}

endpoint&
endpoint::operator=(const ip4_addr& addr)
{ return (*this) = ip4_addr_cast(addr); }

endpoint&
endpoint::operator=(const ip6_addr& addr)
{ return (*this) = ip6_addr_cast(addr); }

endpoint&
endpoint::operator=(const endpoint& other)
{ return (*this) = other.addr_; }

endpoint&
endpoint::operator=(endpoint&& other)
{ return (*this) = other.addr_; }

void
endpoint::set_from_local(int fh)
{
    uint32_t s = size();
    getsockname(fh, &addr_, &s);
}

void
endpoint::set_from_remote(int fh)
{
    uint32_t s = size();
    getpeername(fh, &addr_, &s);
}

std::string
endpoint::ip() const
{
    std::string a;

    to_ip_name(&addr_, a);

    return a;
}

std::size_t
endpoint::size() const
{
    std::size_t s = 0;

    if (addr_.sa_family == AF_INET) {
        s = sizeof(struct sockaddr_in);
    } else if (addr_.sa_family == AF_INET6) {
        s = sizeof(struct sockaddr_in6);
    }

    return s;
}

uint16_t
endpoint::port() const
{
    uint16_t p = 0;

    if (addr_.sa_family == AF_INET) {
        p = htons(ip4_addr_cast(addr_).sin_port);
    } else if (addr_.sa_family == AF_INET6) {
        p = htons(ip6_addr_cast(addr_).sin6_port);
    }

    return p;
}

std::string
endpoint::str() const
{
    std::ostringstream oss;

    oss << ip() << ":" << port();

    return oss.str();
}

endpoint::operator ip_addr_ptr() const
{ return (ip_addr_ptr) &addr_; }

endpoint::operator std::string() const
{ return str(); }

} // namespace nx
