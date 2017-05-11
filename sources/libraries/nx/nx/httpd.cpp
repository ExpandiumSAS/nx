#include <iostream>

#include <nx/httpd.hpp>

namespace nx {

route&
httpd::operator()(const method& m)
{
    auto& r = routes_map_[m.str];

    r.emplace_back();

    return r.back();
}

route&
httpd::operator()(const ws_tag& t)
{
    auto& r = routes_map_["GET"];

    r.emplace_back();

    r.back() = [](const request& req, buffer& data, reply& rep) {};

    return r.back();
}

endpoint
httpd::operator()(const endpoint& ep)
{
    if (ep.ep_protocol == endpoint::protocol::TCP) {
        return
            (*this)(
                ep.ep_tcp
            );
    } else {
        return
            (*this)(
                ep.ep_local
            );
    }
}

endpoint_tcp
httpd::operator()(const endpoint_tcp& ep)
{
    return
        serve(
            s_,
            ep,
            [this](request& req, buffer& data, reply& rep) {
                (*this)(req, data, rep);
            }
        );
}

endpoint_local
httpd::operator()(const endpoint_local& ep)
{
    return
        serve(
            local_s_,
            ep,
            [this](request& req, buffer& data, reply& rep) {
                (*this)(req, data, rep);
            }
        );
}

httpd&
httpd::operator<<(json_collection_base& c)
{
    auto& me = *this;

    // Operations on the whole collection
    collection_tag ct;

    me(GET) / c.path() = c.GET(ct);
    me(POST) / c.path() = c.POST(ct);

    // Operations on a specific item
    item_tag it;

    me(GET) / c.path() / ":id" = c.GET(it);
    me(PUT) / c.path() / ":id" = c.PUT(it);
    me(DELETE) / c.path() / ":id" = c.DELETE(it);

    return me;
}

void
httpd::operator()(request& req, buffer& data, reply& rep)
{
    auto it = routes_map_.find(req.method());

    if (it == routes_map_.end()) {
        // No handlers declared for this method
        throw NotFound;
    }

    auto& routes = it->second;
    bool matched = false;
    std::vector<std::string> matches;

    for (const auto& r : routes) {
        if (r.match(req)) {
            matches.emplace_back(r.path());

            if (!matched) {
                matched = true;
                r(req, data, rep);
            }
        }
    }

    if (!matched) {
        throw NotFound;
    } else if (matches.size() > 1) {
        std::cerr
            << "WHOAA: more than one route matched: " << req.path() << "\n"
            ;

        for (const auto& m : matches) {
            std::cerr << m << std::endl;
        }
    }
}

} // namespace nx
