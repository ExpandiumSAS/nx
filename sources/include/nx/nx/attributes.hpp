#ifndef __NX_ATTRIBUTES_H__
#define __NX_ATTRIBUTES_H__

#include <ostream>
#include <string>
#include <unordered_map>

#include <nx/config.h>

namespace nx {

struct NX_API attribute_base
{
    attribute_base(const std::string& n, const std::string& v);
    attribute_base(std::string&& n, std::string&& v);
    attribute_base(const std::string& n, const char* v);

    template <typename T>
    attribute_base(const std::string& n, const T& v)
    : attribute_base(n, std::to_string(v))
    {}

    std::ostream& operator()(std::ostream& os) const;

    std::string name;
    std::string value;
};

class NX_API attribute_map
{
public:
    using map_type = std::unordered_map<std::string, std::string>;
    using iterator = map_type::iterator;
    using const_iterator = map_type::const_iterator;

    attribute_map(char sep = ';');
    attribute_map(const std::string& data, char sep = ';');
    attribute_map(const attribute_map& other);
    attribute_map(attribute_map&& other);
    virtual ~attribute_map();

    attribute_map& operator=(const attribute_map& other);
    attribute_map& operator=(attribute_map&& other);

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    bool has(const std::string& name) const;

    std::string& operator[](const std::string& name);
    const std::string& operator[](const std::string& name) const;

    attribute_map& operator<<(const attribute_base& a);
    attribute_map& operator<<(attribute_base&& a);
    attribute_map& operator<<(const attribute_map& other);
    attribute_map& operator<<(attribute_map&& other);

    virtual std::ostream& operator()(std::ostream& os) const;

protected:
    char sep_;
    map_type m_;
    map_type lcm_;
    std::string empty_;
};

inline
std::ostream&
operator<<(std::ostream& os, const attribute_map& a)
{
    a(os);
    return os;
}

class attributes : public attribute_map
{
public:
    using attribute_map::attribute_map;
};

struct attribute : public attribute_base
{
    using attribute_base::attribute_base;
};

} // namespace nx

#endif // __NX_ATTRIBUTES_H__
