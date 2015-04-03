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
    try {
        v_ = jsonv::parse(jsonv::string_view(b.data(), b.size()));
        b.clear();
    } catch (const jsonv::parse_error& e) {
        throw BadRequest;
    }
}

jsonv::value&&
json::value() &&
{ return std::move(v_); }

jsonv::value&
json::value() &
{ return v_; }

const jsonv::value&
json::value() const &
{ return v_; }

void
json::operator()(std::ostream& os) const
{ os << v_; }

} // namesapce nx
