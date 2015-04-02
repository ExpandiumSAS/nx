#include <nx/json.hpp>
#include <nx/http_status.hpp>

namespace nx {

json_types&
json_types::get()
{
    static json_types jt;
    return jt;
}

json_types::json_types()
{}

json_types::~json_types()
{}

json::json()
{}

json::json(buffer& b)
{
    std::string js;

    try {
        b >> js;
        v_ = jsonv::parse(js);
    } catch (const jsonv::parse_error& e) {
        throw BadRequest;
    }
}

void
json::operator()(std::ostream& os) const
{ os << v_; }

} // namesapce nx
