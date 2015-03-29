#ifndef __NX_ENDPOINT_H__
#define __NX_ENDPOINT_H__

#include <stdint.h>

#include <ostream>
#include <string>

#include <nx/config.h>
#include <nx/ip.hpp>

namespace nx {

class NX_API endpoint
{
public:
    endpoint();
    explicit endpoint(const char* ip, uint16_t port = 0);
    explicit endpoint(const std::string& ip, uint16_t port = 0);
    explicit endpoint(const ip_addr& addr);
    explicit endpoint(const ip4_addr& addr);
    explicit endpoint(const ip6_addr& addr);
    endpoint(const endpoint& other);
    endpoint(endpoint&& other);

    endpoint& operator=(const ip_addr& addr);
    endpoint& operator=(const ip4_addr& addr);
    endpoint& operator=(const ip6_addr& addr);
    endpoint& operator=(const endpoint& other);
    endpoint& operator=(endpoint&& other);

    void set_from_local(int fh);
    void set_from_remote(int fh);

    std::string ip() const;
    uint16_t port() const;

    std::string str() const;
    std::size_t size() const;

    operator ip_addr_ptr() const;
    operator std::string() const;

private:
    ip_addr addr_;
};

inline
std::ostream&
operator<<(std::ostream& os, const endpoint& ep)
{
    os << ep.str();
    return os;
}

} // namespace nx

#endif // __NX_ENDPOINT_H__
