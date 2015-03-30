#include <nx/headers.hpp>
#include <nx/escape.hpp>

namespace nx {

std::ostream&
headers::operator()(std::ostream& os) const
{
    for (const auto& p : m_) {
        os << p.first << ": " << p.second << "\r\n";
    }

    return os;
}

} // namespace nx
