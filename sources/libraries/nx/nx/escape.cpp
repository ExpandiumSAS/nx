#include <cctype>

#include <regex>

#include <hxx/escape.hpp>
#include <hxx/utils.hpp>

namespace hxx {

std::string
escape(const std::string& s)
{
    static const char* dec2hex = "0123456789ABCDEF";
    static std::regex unsafe("[^A-Za-z0-9\\-\\._~]");

    std::string escaped;
    auto cur = std::sregex_token_iterator(s.begin(), s.end(), unsafe);
    auto end = std::sregex_token_iterator();
    auto vbegin = s.begin();

    for ( ; cur != end; ++cur) {
        // Append valid chars
        escaped.append(vbegin, cur->first);

        for (auto it = cur->first; it != cur->second; ++it) {
            auto c = (unsigned char) ::toupper(*it);

            escaped += '%';
            escaped += dec2hex[c >> 4];
            escaped += dec2hex[c & 0x0F];
        }

        vbegin = cur->second;
    }

    escaped.append(vbegin, s.end());

    return escaped;
}

unsigned char
xdigit_to_num(unsigned char c)
{ return (c < 'A' ? c - '0' : toupper(c) - 'A' + 10); }

std::string
unescape(const std::string& s)
{
    static std::regex escaped("%([0-9A-Fa-f]{2})");

    // Support urlencoded '+' --> ' '
    std::string t = subst(s, "\\+", " ");

    std::string unescaped;
    auto cur = std::sregex_token_iterator(t.begin(), t.end(), escaped);
    auto end = std::sregex_token_iterator();
    auto vbegin = t.cbegin();

    for ( ; cur != end; ++cur) {
        auto it = cur->first;

        // Append unescaped chars
        unescaped.append(vbegin, it);

        // Skip percent
        ++it;

        auto c = xdigit_to_num(*it++) << 4;
        c |= xdigit_to_num(*it++);

        unescaped += c;

        vbegin = cur->second;
    }

    unescaped.append(vbegin, t.cend());

    return unescaped;
}

} // namespace hxx
