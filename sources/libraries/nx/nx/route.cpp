#include <iostream>

#include <nx/route.hpp>
#include <nx/utils.hpp>
#include <nx/attributes.hpp>

namespace nx {

route&
route::operator/(const char* path)
{
    path_ += '/';
    path_ += path;

    return *this;
}

route&
route::operator/(std::string& path)
{
    path_ += '/';
    path_ += path;

    return *this;
}

route&
route::operator=(route_cb cb)
{
    route_cb_ = cb;

    return *this;
}

const std::string&
route::path() const
{ return path_; }

bool
route::match(request& req) const
{
    auto req_toks = split("/", req.path());
    auto my_toks = split("/", path_);

    if (req_toks.size() != my_toks.size()) {
        return false;
    }

    attributes a;

    for (std::size_t i = 0; i < my_toks.size(); i++) {
        auto& mt = my_toks[i];
        auto& rt = req_toks[i];

        if (mt.empty() || rt.empty()) {
            if (i == 0) {
                // Leading '/'
                continue;
            } else {
                // C'mon...
                return false;
            }
        }

        if (mt[0] == ':') {
            // This is a placeholder
            mt.erase(0, 1);
            a << attribute(mt, rt);
        } else if (rt != mt) {
            // Path element mismatch
            return false;
        }
    }

    req << a;

    return true;
}

void
route::operator()(const request& req, buffer& data, reply& rep) const
{ route_cb_(req, data, rep); }

} // namespace nx
