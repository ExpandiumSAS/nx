#ifndef __NX_IP_H__
#define __NX_IP_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

namespace nx {

typedef sockaddr ip_addr;
typedef ip_addr* ip_addr_ptr;
typedef sockaddr_in ip4_addr;
typedef ip4_addr* ip4_addr_ptr;
typedef sockaddr_in6 ip6_addr;
typedef ip6_addr* ip6_addr_ptr;

inline
ip_addr_ptr
ip4_addr_cast(ip4_addr_ptr p)
{ return reinterpret_cast<ip_addr_ptr>(p); }

inline
ip_addr&
ip4_addr_cast(ip4_addr& p)
{ return reinterpret_cast<ip_addr&>(p); }

inline
const ip_addr&
ip4_addr_cast(const ip4_addr& p)
{ return reinterpret_cast<const ip_addr&>(p); }

inline
ip4_addr_ptr
ip4_addr_cast(ip_addr_ptr p)
{ return reinterpret_cast<ip4_addr_ptr>(p); }

inline
ip4_addr&
ip4_addr_cast(ip_addr& p)
{ return reinterpret_cast<ip4_addr&>(p); }

inline
const ip4_addr&
ip4_addr_cast(const ip_addr& p)
{ return reinterpret_cast<const ip4_addr&>(p); }

inline
ip_addr_ptr
ip6_addr_cast(ip6_addr_ptr p)
{ return reinterpret_cast<ip_addr_ptr>(p); }

inline
ip_addr&
ip6_addr_cast(ip6_addr& p)
{ return reinterpret_cast<ip_addr&>(p); }

inline
const ip_addr&
ip6_addr_cast(const ip6_addr& p)
{ return reinterpret_cast<const ip_addr&>(p); }

inline
ip6_addr_ptr
ip6_addr_cast(ip_addr_ptr p)
{ return reinterpret_cast<ip6_addr_ptr>(p); }

inline
ip6_addr&
ip6_addr_cast(ip_addr& p)
{ return reinterpret_cast<ip6_addr&>(p); }

inline
const ip6_addr&
ip6_addr_cast(const ip_addr& p)
{ return reinterpret_cast<const ip6_addr&>(p); }

} // namespace nx

#endif // __NX_IP_H__
