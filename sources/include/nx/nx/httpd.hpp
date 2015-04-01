#ifndef __NX_HTTPD_H__
#define __NX_HTTPD_H__

#include <string>

#include <nx/config.h>
#include <nx/http.hpp>
#include <nx/methods.hpp>
#include <nx/route.hpp>
#include <nx/json_collection.hpp>

namespace nx {

class NX_API httpd
{
public:
    route& operator()(const method& m);

    const endpoint& operator()(const endpoint& ep);

    template <typename T>
    httpd& operator<<(json_collection<T> c)
    {
        auto& me = *this;

        // Operations on the whole collection
        collection_tag ct;

        me(GET) / c.path() = c.GET(ct);
        me(PUT) / c.path() = c.PUT(ct);
        me(POST) / c.path() = c.POST(ct);
        me(DELETE) / c.path() = c.DELETE(ct);

        // Operations on a specific item
        item_tag it;

        me(GET) / c.path() / ":id" = c.GET(it);
        me(PUT) / c.path() / ":id" = c.PUT(it);
        me(DELETE) / c.path() / ":id" = c.DELETE(it);

        return me;
    }

private:
    void operator()(request& req, buffer& data, reply& rep);

    http s_;
    routes_map routes_map_;
};

} // namespace nx

#endif // __NX_HTTPD_H__
