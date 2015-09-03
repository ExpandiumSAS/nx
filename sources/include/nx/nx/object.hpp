#ifndef __NX_OBJECT_H__
#define __NX_OBJECT_H__

#include <nx/object_base.hpp>
#include <nx/handle_error.hpp>
#include <nx/handlers.hpp>
#include <nx/service.hpp>

namespace nx {

struct postponer
{
    postponer& operator<<(void_cb&& cb)
    {
        auto ptr = o.ptr();

        async() << [ptr, cb = std::move(cb)]() {
            cb();
        };

        return *this;
    }

    object_base& o;
};

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

    void dispose()
    {
        auto self = ptr();

        async() << [self]() {
            service::get().remove(self);
        };
    }

    postponer postpone()
    { return postponer{ *this }; }

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
