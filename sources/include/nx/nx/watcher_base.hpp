#ifndef __NX_WATCHER_BASE_H__
#define __NX_WATCHER_BASE_H__

#include <cstring>
#include <functional>

#include <nx/loop.hpp>
#include <nx/callback_access.hpp>
#include <nx/watcher_cast.hpp>

namespace nx {

template <typename Derived, typename Watcher>
class watcher_base
{
public:
    using watcher_type = Watcher;
    using this_type = watcher_base<Derived, Watcher>;
    using event_cb = std::function<void(Derived&, int)>;
    using set_cb = std::function<void()>;

    struct on_events {};

    watcher_base() noexcept
    : cb_(nullptr)
    {
        std::memset((void*) &w_, 0, sizeof(watcher_type));
        ev_init(&w_, 0);
        w_.data = static_cast<void*>(this);
    }

    watcher_base(const this_type& other) = delete;

    virtual ~watcher_base()
    { stop(); }

    virtual void start() noexcept
    {}

    virtual void stop() noexcept
    {}

    this_type& operator=(const this_type& other) = delete;

    this_type& operator=(event_cb cb)
    {
        cb_ = cb;

        ev_set_cb(
            ptr(),
            [](auto l, watcher_type* w, int revents) {
                auto& me = watcher_cast<this_type>(w);
                callback_access::call<on_events>(me, revents);
            }
        );

        return *this;
    }

    bool is_active() const noexcept
    { return ev_is_active(ptr()); }

    bool is_pending() const noexcept
    { return ev_is_pending(ptr()); }

    watcher_type& w()
    { return w_; }

    const watcher_type& w() const
    { return w_; }

    watcher_type* ptr()
    { return &w_; }

    const watcher_type* ptr() const
    { return &w_; }

protected:
    void set(set_cb cb) noexcept
    {
        bool active = is_active();

        if (active) {
            stop();
        }

        cb();

        if (active) {
            start();
        }
    }

private:
    friend callback_access;

    void operator()(const on_events& tag, int revents)
    { cb_(derived(), revents); }

    // CRTP interface
    Derived& derived()
    { return *static_cast<Derived* const>(this); }
    Derived const& derived() const
    { return *static_cast<Derived const*>(this); }

    event_cb cb_;
    watcher_type w_;
};

} // namespace nx

#endif // __NX_WATCHER_BASE_H__
