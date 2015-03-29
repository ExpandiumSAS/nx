#ifndef __NX_CALLBACKS_H__
#define __NX_CALLBACKS_H__

#include <functional>
#include <tuple>
#include <type_traits>

#include <nx/tuple_utils.hpp>
#include <nx/utils.hpp>

namespace nx {

struct callback_tag {};

template <typename Tag, typename... Args>
class callback
{
public:
    static_assert(
        std::is_base_of<callback_tag, Tag>::value,
        "invalid callback tag"
    );

    using this_type = callback<Tag, Args...>;
    using tag_type = Tag;
    using type = std::function<void(Args...)>;

    operator bool() const
    { return (bool) cb_; }

    operator type() const
    { return cb_; }

    bool operator()(Args&&... args)
    {
        bool called = false;

        if (cb_) {
            cb_(std::forward<Args>(args)...);
            called = true;
        }

        return called;
    }

    void reset()
    { cb_ = nullptr; }

    this_type& operator=(const type& cb)
    {
        cb_ = cb;

        return *this;
    }

    this_type& operator=(type&& cb)
    {
        cb_ = std::move(cb);

        return *this;
    }

private:
    type cb_;
};

template <typename... Callbacks>
class callbacks
{
public:
    using callbacks_type = std::tuple<Callbacks...>;
    using tags_type = std::tuple<typename Callbacks::tag_type...>;

    void clear()
    {
        nx::for_each(
            cbs_,
            [](std::size_t pos, auto& cb) {
                cb.reset();
            }
        );
    }

    template <typename Tag>
    auto&
    get(const Tag& tag)
    { return get<Tag>(cbs_); }

    template <typename Tag>
    bool has(const Tag& tag) const
    { return (bool) get<Tag>(cbs_); }

    template <typename Tag, typename... Args>
    bool
    call(Args&&... args)
    { return get<Tag>(cbs_)(std::forward<Args>(args)...); }

private:
    template <typename Tag, typename Functions>
    typename nx::tuple_element<
        Tag,
        Functions,
        tags_type
    >::type&
    get(Functions& functions)
    {
        using function_type = typename nx::tuple_element<
            Tag,
            Functions,
            tags_type
        >::type;

        return std::get<function_type>(functions);
    }

    template <typename Tag, typename Functions>
    const typename nx::tuple_element<
        Tag,
        Functions,
        tags_type
    >::type&
    get(const Functions& functions) const
    {
        using function_type = typename nx::tuple_element<
            Tag,
            Functions,
            tags_type
        >::type;

        return std::get<function_type>(functions);
    }

    callbacks_type cbs_;
};

template <typename Tag, typename Callbacks>
struct callback_element
{
    using callbacks_type = typename Callbacks::callbacks_type;
    using tags_type = typename Callbacks::tags_type;

    using type = typename nx::tuple_element<
        Tag,
        callbacks_type,
        tags_type
    >::type;
};

template <typename Tag, typename Class>
struct callback_signature
{
    using type = typename callback_element<
        Tag,
        typename Class::callbacks
    >::type::type;
};

} // namespace nx

#endif // __NX_CALLBACKS_H__
