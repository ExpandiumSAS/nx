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
        throw std::runtime_error(e.what());
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

void
check_paths(const jsonv::value& v, std::initializer_list<std::string> paths)
{
    for (const auto& p : paths) {
        if (v.count_path(jsonv::path::create(p)) == 0) {
            throw std::runtime_error("json path not found: " + p);
        }
    }
}

} // namesapce nx
