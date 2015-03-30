#include <hxx/headers.hpp>
#include <hxx/escape.hpp>

namespace hxx {

std::ostream&
headers::operator()(std::ostream& os) const
{
    for (const auto& p : m_) {
        os << p.first << ": " << p.second << "\r\n";
    }

    return os;
}

} // namespace hxx
