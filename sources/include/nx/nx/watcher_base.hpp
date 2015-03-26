#ifndef __NX_WATCHER_BASE_H__
#define __NX_WATCHER_BASE_H__

#include <ev.h>

#include <functional>

#include <nx/callback_access.hpp>
#include <nx/watcher_cast.hpp>

namespace nx {

template <typename Derived, typename Watcher>
class watcher_base : public Watcher
{
public:
    using base_type = Watcher;
    using this_type = watcher_base<Derived, Watcher>;
    using cb_type = std::function<void(Derived&, int)>;

    watcher_base() noexcept
    {
        this->data = static_cast<void*>(this);
        ev_init(this, 0);
    }

    watcher_base(const this_type& other) = delete;

    virtual ~watcher_base()
    { stop(); }

    virtual void start() = 0;
    virtual void stop() = 0;

    this_type& operator=(cb_type cb)
    {
        cb_ = cb;

        ev_set_cb(
            *this,
            [](base_type* w, int revents) {
                auto& me = watcher_cast<this_type>(w);
                callback_access::call(me, revents);
            }
        );

        return *this;
    }

    bool is_active() const noexcept
    { return ev_is_active(*this); }

    bool is_pending() const noexcept
    { return ev_is_pending(*this); }

    operator base_type*()
    { return static_cast<base_type*>(this); }

private:
    friend callback_access;

    void operator()(int revents)
    { cb_(derived(), revents); }

    // CRTP interface
    Derived& derived()
    { return *static_cast<Derived* const>(this); }
    Derived const& derived() const
    { return *static_cast<Derived const*>(this); }

    cb_type cb_;
};

} // namespace nx

#endif // __NX_WATCHER_BASE_H__
