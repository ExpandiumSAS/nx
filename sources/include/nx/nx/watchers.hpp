#ifndef __NX_WATCHERS_H__
#define __NX_WATCHERS_H__

#include <nx/config.h>
#include <nx/watcher_base.hpp>

namespace nx {

class io : public watcher_base<io, ev_io>
{
public:
    using base_type = watcher_base<io, ev_io>;

    virtual void start() noexcept
    { loop::get()([&]() { ev_io_start(ptr()); }); }

    virtual void stop() noexcept
    { loop::get()([&]() { ev_io_stop(ptr()); }); }

    io& operator()(int fd, int events) noexcept
    {
        set([&](){ ev_io_set(ptr(), fd, events); });
        return *this;
    }

    io& operator()(int events) noexcept
    { return (*this)(fd, events); }

    void start(int fd, int events) noexcept
    {
        (*this)(fd, events);
        start();
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

    virtual void start() noexcept
    { loop::get()([&]() { ev_timer_start(ptr()); }); }

    virtual void stop() noexcept
    { loop::get()([&]() { ev_timer_stop(ptr()); }); }

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
    { loop::get()([&]() { ev_timer_again(ptr()); }); }

    timestamp remaining()
    {
        timestamp t;

        loop::get()([&]() { t = ev_timer_remaining(ptr()); });

        return t;
    }

    timer& operator=(event_cb cb)
    {
        base_type::operator=(cb);
        return *this;
    }
};

} // namespace nx

#endif // __NX_WATCHERS_H__
