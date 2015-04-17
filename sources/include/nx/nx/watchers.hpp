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
    using base_type::start;
    using base_type::stop;
    using base_type::operator=;

    io(int fd)
    : base_type(ev_io_start, ev_io_stop)
    {
        w().fd = fd;
        (*this)(0);
    }

    io& operator()(int events) noexcept
    {
        modify([&,events](){ ev_io_set(ptr(), w().fd, events); });

        return *this;
    }

    io& operator|=(int events) noexcept
    { return (*this)(w().events | events); }

    io& operator^=(int events) noexcept
    { return (*this)(w().events & ~events); }
};

class timer : public watcher_base<timer, ev_timer>
{
public:
    using base_type = watcher_base<timer, ev_timer>;
    using base_type::start;
    using base_type::stop;
    using base_type::operator=;

    timer()
    : base_type(ev_timer_start, ev_timer_stop)
    {}

    timer& operator()(timestamp after, timestamp repeat = 0.) noexcept
    {
        modify([&,after,repeat](){ ev_timer_set(ptr(), after, repeat); });
        return *this;
    }

    void start(timestamp after, timestamp repeat = 0.) noexcept
    {
        modify([&,after,repeat](){ ev_timer_set(ptr(), after, repeat); });
        start();
    }

    void again() noexcept
    { async() << [&](evloop el) { ev_timer_again(el, ptr()); }; }
};

} // namespace nx

#endif // __NX_WATCHERS_H__
