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

const endpoint&
httpd::operator()(const endpoint& ep)
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

void
httpd::operator()(request& req, buffer& data, reply& rep)
{
    auto it = routes_map_.find(req.method());

    if (it == routes_map_.end()) {
        // No handlers declared for this method
        throw not_found();
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
        throw not_found();
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
