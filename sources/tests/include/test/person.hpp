#ifndef __NX_TEST_PERSON_H__
#define __NX_TEST_PERSON_H__

#include <string>

#include <jsonv/serialization.hpp>

namespace test {

struct person {
    std::string id;
    std::string name;
    std::size_t age;
};

auto person_fmt =
    jsonv::formats_builder()
    .type<person>()
        .member("id", &person::id)
        .member("name", &person::name)
        .member("age", &person::age)
    .check_references(jsonv::formats::defaults())
    ;

} // namespace test

namespace std {

// STL container support
template <>
struct hash<test::person>
{
    using argument_type = test::person;
    using result_type = std::size_t;

    std::size_t operator()(const test::person& x) const
    { return std::hash<std::string>()(x.id); }
};

template <>
struct equal_to<test::person>
{
    using result_type = bool;
    using first_argument_type = test::person;
    using second_argument_type = test::person;

    bool operator()(
        const test::person& lhs,
        const test::person& rhs
    ) const
    {
        return lhs.id == rhs.id;
    }
};

} // namespace std

#endif // __NX_TEST_PERSON_H__
