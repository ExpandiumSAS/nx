#ifndef __NX_ROUTE_H__
#define __NX_ROUTE_H__

#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include <nx/config.h>
#include <nx/request.hpp>
#include <nx/reply.hpp>

namespace nx {

class NX_API route
{
public:
    using route_cb = std::function<
        void(const request& req, buffer& data, reply& rep)
    >;

    route& operator/(const char* path);
    route& operator/(std::string& path);
    route& operator=(route_cb cb);

    const std::string& path() const;

    bool match(request& req) const;

    void operator()(const request& req, buffer& data, reply& rep) const;

private:
    std::string path_;
    route_cb route_cb_;
};

using routes = std::vector<route>;
using routes_map = std::unordered_map<std::string, routes>;

} // namespace nx

#endif // __NX_ROUTE_H__
