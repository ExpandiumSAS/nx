#include <nx/attributes.hpp>
#include <nx/escape.hpp>
#include <nx/utils.hpp>

namespace nx {

attribute_base::attribute_base(const std::string& n, const std::string& v)
: name(n),
value(v)
{}

attribute_base::attribute_base(std::string&& n, std::string&& v)
: name(std::move(n)),
value(std::move(v))
{}

std::ostream&
attribute_base::operator()(std::ostream& os) const
{
    os << name << "=" << escape(value);
    return os;
}

attribute_map::attribute_map(char sep)
: sep_(sep)
{}

attribute_map::attribute_map(const std::string& data, char sep)
: sep_(sep)
{
    std::string re = "\\s*";
    re.push_back(sep_);
    re += "\\s*";

    for (auto& a : split(re, data)) {
        auto i = a.find('=');

        if (i == std::string::npos || i == 0) {
            continue;
        }

        auto var = unescape(a.substr(0, i));
        auto tval = a.substr(i + 1);
        auto it = tval.find('"');

        while (it != std::string::npos) {
            tval.erase(it);
            it = tval.find('"');
        }

        auto val = unescape(tval);

        lcm_.emplace(lc(var), var);
        m_.emplace(std::move(var), std::move(val));
    }
}

attribute_map::attribute_map(const attribute_map& other)
{ *this = other; }

attribute_map::attribute_map(attribute_map&& other)
{ *this = std::move(other); }

attribute_map::~attribute_map()
{}

attribute_map&
attribute_map::operator=(const attribute_map& other)
{
    sep_ = other.sep_;
    m_ = other.m_;
    lcm_ = other.lcm_;

    return *this;
}

attribute_map&
attribute_map::operator=(attribute_map&& other)
{
    sep_ = other.sep_;
    m_ = std::move(other.m_);
    lcm_ = std::move(other.lcm_);

    return *this;
}

attribute_map::iterator
attribute_map::begin()
{ return m_.begin(); }

attribute_map::const_iterator
attribute_map::begin() const
{ return m_.begin(); }

attribute_map::iterator
attribute_map::end()
{ return m_.end(); }

attribute_map::const_iterator
attribute_map::end() const
{ return m_.end(); }

bool
attribute_map::has(const std::string& name) const
{
    return
        m_.find(name) != m_.end()
        ||
        lcm_.find(name) != lcm_.end()
        ;
}

std::string&
attribute_map::operator[](const std::string& name)
{
    auto it = m_.find(name);

    if (it != m_.end()) {
        return it->second;
    }

    it = lcm_.find(name);

    if (
        it == lcm_.end()
        ||
        (it = m_.find(it->second)) == m_.end()
    ) {
        throw std::runtime_error("no such attribute: " + name);
    }

    return it->second;
}

const std::string&
attribute_map::operator[](const std::string& name) const
{
    auto it = m_.find(name);

    if (it != m_.end()) {
        return it->second;
    }

    it = lcm_.find(name);

    if (it != lcm_.end()) {
        return m_.at(it->second);
    }

    return empty_;
}

attribute_map&
attribute_map::operator<<(const attribute_base& a)
{
    lcm_.emplace(lc(a.name), a.name);
    m_.emplace(a.name, a.value);

    return *this;
}

attribute_map&
attribute_map::operator<<(attribute_base&& a)
{
    lcm_.emplace(lc(a.name), a.name);
    m_.emplace(std::move(a.name), std::move(a.value));

    return *this;
}

attribute_map&
attribute_map::operator<<(const attribute_map& other)
{
    for (const auto& p : other.m_) {
        m_.emplace(p);
    }

    for (const auto& p : other.lcm_) {
        lcm_.emplace(p);
    }

    return *this;
}

attribute_map&
attribute_map::operator<<(attribute_map&& other)
{
    for (auto& p : other.m_) {
        m_.emplace(std::move(p));
    }

    for (auto& p : other.lcm_) {
        lcm_.emplace(std::move(p));
    }

    return *this;
}

std::ostream&
attribute_map::operator()(std::ostream& os) const
{
    bool first = true;

    for (const auto& p : m_) {
        if (!first) {
            first = false;
            os << sep_;
        }

        os << p.first << '=' << escape(p.second);
    }

    return os;
}

} // namespace nx
