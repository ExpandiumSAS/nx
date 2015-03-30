#include <hxx/json.hpp>

namespace hxx {

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

    b >> js;
    v_ = jsonv::parse(js);
}

void
json::operator()(std::ostream& os) const
{ os << v_; }

} // namesapce hxx
