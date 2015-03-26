#ifndef __NX_HANDLE_BASE_H__
#define __NX_HANDLE_BASE_H__

#include <type_traits>

#include <nx/config.h>

#include <nx/callbacks.hpp>
#include <nx/buffer.hpp>

namespace nx {

namespace tags {

struct on_error_tag : callback_tag {};
struct on_read_tag : callback_tag {};
struct on_drain_tag : callback_tag {};
struct on_eof_tag : callback_tag {};

const on_error_tag on_error = {};
const on_read_tag on_read = {};
const on_drain_tag on_drain = {};
const on_eof_tag on_eof = {};

} // namespace tags

template <typename Derived, typename... Callbacks>
class handle_base
{
public:
    using callbacks = nx::callbacks<
        callback<tags::on_error_tag, Derived&, const char*>,
        callback<tags::on_read_tag, Derived&, buffer&>,
        callback<tags::on_eof_tag, Derived&>,
        callback<tags::on_drain_tag, Derived&>,
        Callbacks...
    >;

    handle_base(int fh) noexcept
    : fh_(fh)
    {}

    virtual ~handle_base()
    {}

    template <
        typename Tag,
        typename Enabled = typename std::enable_if<
            std::is_base_of<callback_tag, Tag>::value
        >::type
    >
    auto&
    handler(const Tag& t)
    { return callbacks_.get(t); }

    template <typename Tag>
    auto&
    operator[](const Tag& t)
    { return handler(t); }

private:
    /// CRTP interface
    Derived* derived_this()
    { return static_cast<Derived*>(this); }

    Derived& derived()
    { return *static_cast<Derived* const>(this); }
    Derived const& derived() const
    { return *static_cast<Derived const*>(this); }

    int fh_;
    callbacks callbacks_;
};

} // namespace nx

#endif // __NX_HANDLE_BASE_H__
