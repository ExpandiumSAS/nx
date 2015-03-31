#ifndef __NX_WATCHERS_H__
#define __NX_WATCHERS_H__

#include <nx/config.h>
#include <nx/watcher_base.hpp>
#include <nx/cond_var.hpp>

namespace nx {

class io : public watcher_base<io, ev_io>
{
public:
    using base_type = watcher_base<io, ev_io>;
    using base_type::operator=;

    io(int fd)
    : base_type()
    { set([&](){ ev_io_set(ptr(), fd, 0); }); }

    virtual void start() noexcept
    { async() << [&](evloop el) { ev_io_start(el, ptr()); }; }

    virtual void stop() noexcept
    { async() << [&](evloop el) { ev_io_stop(el, ptr()); }; }

    virtual void sync_stop() noexcept
    {
        cond_var stopped;

        async() << [&](evloop el) {
            ev_io_stop(el, ptr());
            std::cout << std::this_thread::get_id() << " NOTIFY" << std::endl;
            stopped.notify();
        };

        std::cout << std::this_thread::get_id() << " WAIT" << std::endl;
        stopped.wait();
    }

    io& operator()(int fd, int events) noexcept
    {
        set([&](){ ev_io_set(ptr(), fd, events); });
        return *this;
    }

    io& operator()(int events) noexcept
    { return (*this)(w().fd, events); }

    io& operator|=(int events) noexcept
    { return (*this)(w().fd, w().events | events); }

    io& operator^=(int events) noexcept
    { return (*this)(w().fd, w().events & ~events); }

    void start(int fd, int events) noexcept
    {
        (*this)(fd, events);
        start();
    }
};

class timer : public watcher_base<timer, ev_timer>
{
public:
    using base_type = watcher_base<timer, ev_timer>;
    using base_type::operator=;

    timer()
    : base_type()
    {}

    virtual void start() noexcept
    { async() << [&](evloop el) { ev_timer_start(el, ptr()); }; }

    virtual void stop() noexcept
    { async() << [&](evloop el) { ev_timer_stop(el, ptr()); }; }

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
    { async() << [&](evloop el) { ev_timer_again(el, ptr()); }; }
};

} // namespace nx

#endif // __NX_WATCHERS_H__
