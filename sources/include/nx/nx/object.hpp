#ifndef __NX_OBJECT_H__
#define __NX_OBJECT_H__

#include <iostream>
#include <memory>

#include <nx/config.h>
#include <nx/buffer.hpp>
#include <nx/handle_error.hpp>

namespace nx {

class NX_API object_base
: public std::enable_shared_from_this<object_base>
{
public:
    object_base() = default;
    object_base(const object_base& other) = delete;
    object_base(object_base&& other) = default;
    object_base& operator=(const object_base& other) = delete;
    object_base& operator=(object_base&& other) = default;

    std::shared_ptr<object_base> ptr()
    {
        auto self(shared_from_this());

        return self;
    }

    virtual void stop() = 0;
};

using object_ptr = std::shared_ptr<object_base>;

template <
    typename Derived,
    typename... Callbacks
>
class object : public object_base
{
public:
    using this_type = object<Derived, Callbacks...>;
    using callbacks = nx::callbacks<
        callback<tags::on_error_tag, Derived&, const error_code&>,
        Callbacks...
    >;

    object() = default;
    object(const object& other) = delete;
    object(object&& other) = default;
    object& operator=(const object& other) = delete;
    object& operator=(object&& other) = default;

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

protected:
    Derived& derived()
    { return *static_cast<Derived* const>(this); }
    Derived const& derived() const
    { return *static_cast<Derived const*>(this); }

private:
    callbacks callbacks_;
};

} // namespace nx

#endif // __NX_OBJECT_H__
