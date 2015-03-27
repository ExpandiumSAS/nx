#ifndef __NX_WATCHERS_H__
#define __NX_WATCHERS_H__

#include <nx/config.h>
#include <nx/watcher_base.hpp>

namespace nx {

class io : public watcher_base<io, ev_io>
{
public:
    using base_type = watcher_base<io, ev_io>;

    io()
    : base_type()
    {}

    virtual void start() noexcept
    { loop::get()([&](evloop el) { ev_io_start(el, ptr()); }); }

    virtual void stop() noexcept
    { loop::get()([&](evloop el) { ev_io_stop(el, ptr()); }); }

    io& operator()(int fd, int events) noexcept
    {
        set([&](){ ev_io_set(ptr(), fd, events); });
        return *this;
    }

    io& operator()(int events) noexcept
    { return (*this)(w().fd, events); }

    void start(int fd, int events) noexcept
    {
        (*this)(fd, events);
        start();
    }

    io& operator=(watcher_event_cb cb)
    {
        base_type::operator=(cb);
        return *this;
    }

    io& operator=(event_cb cb)
    {
        base_type::operator=(cb);
        return *this;
    }
};

class timer : public watcher_base<timer, ev_timer>
{
public:
    using base_type = watcher_base<timer, ev_timer>;

    timer()
    : base_type()
    {}

    virtual void start() noexcept
    { loop::get()([&](evloop el) { ev_timer_start(el, ptr()); }); }

    virtual void stop() noexcept
    { loop::get()([&](evloop el) { ev_timer_stop(el, ptr()); }); }

    timer& operator()(timestamp after, timestamp repeat = 0.) noexcept
    {
        set([&](){ ev_timer_set(ptr(), after, repeat); });
        return *this;
    }

    void start(timestamp after, timestamp repeat = 0.) noexcept
    {
        set([&](){ ev_timer_set(ptr(), after, repeat); });
        start();
    }

    void again() noexcept
    { loop::get()([&](evloop el) { ev_timer_again(el, ptr()); }); }

    timestamp remaining()
    {
        timestamp t;

        loop::get()([&](evloop el) { t = ev_timer_remaining(el, ptr()); });

        return t;
    }

    timer& operator=(watcher_event_cb cb)
    {
        base_type::operator=(cb);
        return *this;
    }

    timer& operator=(event_cb cb)
    {
        base_type::operator=(cb);
        return *this;
    }
};

} // namespace nx

#endif // __NX_WATCHERS_H__
