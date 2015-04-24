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
    using watcher_cb = std::function<void(Derived&)>;
    using watcher_event_cb = std::function<void(Derived&, int)>;
    using event_cb = std::function<void(int)>;
    using watcher_func = void (*)(evloop, watcher_type*);

    struct on_void {};
    struct on_watcher {};
    struct on_watcher_events {};
    struct on_events {};

    watcher_base()
    : start_func_(nullptr),
    stop_func_(nullptr),
    void_cb_(nullptr),
    watcher_cb_(nullptr),
    watcher_event_cb_(nullptr),
    event_cb_(nullptr)
    {}

    watcher_base(
        watcher_func start_func,
        watcher_func stop_func
    ) noexcept
    : start_func_(start_func),
    stop_func_(stop_func),
    void_cb_(nullptr),
    watcher_cb_(nullptr),
    watcher_event_cb_(nullptr),
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
    {}

    this_type& operator=(this_type&& other)
    {
        start_func_ = other.start_func_;
        stop_func_ = other.stop_func_;
        void_cb_ = std::move(other.void_cb_);
        watcher_cb_ = std::move(other.watcher_cb_);
        watcher_event_cb_ = std::move(other.watcher_event_cb_);
        event_cb_ = std::move(other.event_cb_);
        std::memcpy((void*) &w_, (const void*) &other.w_, sizeof(watcher_type));
        w_.data = static_cast<void*>(this);

        return *this;
    }

    void start() noexcept
    { async() << [&](evloop el) { start_func_(el, ptr()); }; }

    void stop() noexcept
    { async() << [&](evloop el) { stop_func_(el, ptr()); }; }

    void stop(void_cb&& on_stopped) noexcept
    {
        async() << [&,on_stopped = std::move(on_stopped)](evloop el) {
            stop_func_(el, ptr());
            on_stopped();
        };
    }

    Derived& operator=(void_cb&& cb)
    {
        void_cb_ = std::move(cb);

        set_cb(
            [](auto l, watcher_type* w, int revents) {
                auto& me = watcher_cast<this_type>(w);
                callback_access::call<on_void>(me);
            }
        );

        return derived();
    }

    Derived& operator=(watcher_cb&& cb)
    {
        watcher_cb_ = std::move(cb);

        set_cb(
            [](auto l, watcher_type* w, int revents) {
                auto& me = watcher_cast<this_type>(w);
                callback_access::call<on_watcher>(me);
            }
        );

        return derived();
    }

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

    void modify(void_cb&& cb) noexcept
    {
        async() << [&,cb = std::move(cb)](evloop el) {
            bool active = is_active();

            if (active) {
                stop_func_(el, ptr());
            }

            cb();

            if (active) {
                start_func_(el, ptr());
            }
        };
    }

private:
    friend callback_access;

    void operator()(const on_void& tag)
    { void_cb_(); }

    void operator()(const on_watcher& tag)
    { watcher_cb_(derived()); }

    void operator()(const on_watcher_events& tag, int revents)
    { watcher_event_cb_(derived(), revents); }

    void operator()(const on_events& tag, int revents)
    { event_cb_(revents); }

    // CRTP interface
    Derived& derived()
    { return *static_cast<Derived* const>(this); }
    Derived const& derived() const
    { return *static_cast<Derived const*>(this); }

    watcher_func start_func_;
    watcher_func stop_func_;
    void_cb void_cb_;
    watcher_cb watcher_cb_;
    watcher_event_cb watcher_event_cb_;
    event_cb event_cb_;
    watcher_type w_;
};

} // namespace nx

#endif // __NX_WATCHER_BASE_H__
