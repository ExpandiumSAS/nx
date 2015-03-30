#ifndef __NX_WATCHER_BASE_H__
#define __NX_WATCHER_BASE_H__

#include <cstring>
#include <functional>

#include <nx/loop.hpp>
#include <nx/event.hpp>
#include <nx/callback_access.hpp>
#include <nx/watcher_cast.hpp>

namespace nx {

template <typename Derived, typename Watcher>
class watcher_base
{
public:
    using watcher_type = Watcher;
    using this_type = watcher_base<Derived, Watcher>;
    using watcher_event_cb = std::function<void(Derived&, int)>;
    using event_cb = std::function<void(int)>;

    struct on_watcher_events {};
    struct on_events {};

    watcher_base() noexcept
    : watcher_event_cb_(nullptr),
    event_cb_(nullptr)
    {
        std::memset((void*) &w_, 0, sizeof(watcher_type));
        ev_init(&w_, 0);
        w_.data = static_cast<void*>(this);
    }

    watcher_base(this_type&& other)
    { *this = std::move(other); }

    watcher_base(const this_type& other) = delete;
    this_type& operator=(const this_type& other) = delete;

    virtual ~watcher_base()
    { stop(); }

    this_type& operator=(this_type&& other)
    {
        watcher_event_cb_ = std::move(other.watcher_event_cb_);
        event_cb_ = std::move(other.event_cb_);
        std::memcpy((void*) &w_, (const void*) &other.w_, sizeof(watcher_type));
        w_.data = static_cast<void*>(this);

        return *this;
    }

    virtual void start() noexcept
    {}

    virtual void stop() noexcept
    {}

    Derived& operator=(watcher_event_cb&& cb)
    {
        watcher_event_cb_ = std::move(cb);

        set_cb(
            [](auto l, watcher_type* w, int revents) {
                auto& me = watcher_cast<this_type>(w);
                callback_access::call<on_watcher_events>(me, revents);
            }
        );

        return derived();
    }

    Derived& operator=(event_cb&& cb)
    {
        event_cb_ = std::move(cb);

        set_cb(
            [](auto l, watcher_type* w, int revents) {
                auto& me = watcher_cast<this_type>(w);
                callback_access::call<on_events>(me, revents);
            }
        );

        return derived();
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
    template <typename Callback>
    Derived& set_cb(Callback&& cb)
    {
        async() << [&]() { ev_set_cb(ptr(), cb); };

        return derived();
    }

    void set(void_cb cb) noexcept
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

    void operator()(const on_watcher_events& tag, int revents)
    { watcher_event_cb_(derived(), revents); }

    void operator()(const on_events& tag, int revents)
    { event_cb_(revents); }

    // CRTP interface
    Derived& derived()
    { return *static_cast<Derived* const>(this); }
    Derived const& derived() const
    { return *static_cast<Derived const*>(this); }

    watcher_event_cb watcher_event_cb_;
    event_cb event_cb_;
    watcher_type w_;
};

} // namespace nx

#endif // __NX_WATCHER_BASE_H__
