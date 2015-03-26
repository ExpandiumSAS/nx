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
    endpoint(const std::string& ip, uint16_t port = 0);
    endpoint(const ip_addr& addr);
    endpoint(const ip4_addr& addr);
    endpoint(const ip6_addr& addr);
    endpoint(const endpoint& other);
    endpoint(endpoint&& other);

    endpoint& operator=(const ip_addr& addr);
    endpoint& operator=(const ip4_addr& addr);
    endpoint& operator=(const ip6_addr& addr);
    endpoint& operator=(const endpoint& other);
    endpoint& operator=(endpoint&& other);

    const std::string& ip() const;
    const char* ip_c_str() const;
    uint16_t port() const;

    std::string str() const;

    operator ip_addr_ptr() const;
    operator std::string() const;

private:
    void init();
    void init(const ip_addr& addr);
    void init(const ip4_addr& addr);
    void init(const ip6_addr& addr);

    std::string ip_;
    uint16_t port_;
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
