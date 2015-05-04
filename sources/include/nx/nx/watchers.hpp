#ifndef __NX_WATCHERS_H__
#define __NX_WATCHERS_H__

#include <nx/config.h>
#include <nx/watcher.hpp>
#include <nx/cond_var.hpp>

namespace nx {

class io : public watcher<io, ev_io>
{
public:
    using base_type = watcher<io, ev_io>;
    using base_type::start;
    using base_type::stop;
    using base_type::w;
    using base_type::operator=;

    io(int fd)
    : base_type(ev_io_start, ev_io_stop)
    {
        w().fd = fd;
        (*this)(0);
    }

    io(io&& other)
    { *this = std::move(other); }

    io(const io& other) = delete;
    io& operator=(const io& other) = delete;

    io& operator=(io&& other)
    {
        base_type::operator=(std::forward<base_type>(other));

        return *this;
    }

    io& operator()(int events) noexcept
    {
        modify([&,events](){ ev_io_set(wptr(), w().fd, events); });

        return *this;
    }

    io& operator|=(int events) noexcept
    { return (*this)(w().events | events); }

    io& operator^=(int events) noexcept
    { return (*this)(w().events & ~events); }
};

class timer : public watcher<timer, ev_timer>
{
public:
    using base_type = watcher<timer, ev_timer>;
    using base_type::start;
    using base_type::stop;
    using base_type::operator=;

    timer()
    : base_type(ev_timer_start, ev_timer_stop)
    {}

    timer(timer&& other)
    { *this = std::move(other); }

    timer(const timer& other) = delete;
    timer& operator=(const timer& other) = delete;

    timer& operator=(timer&& other)
    {
        base_type::operator=(std::forward<base_type>(other));

        return *this;
    }

    timer& operator()(timestamp after, timestamp repeat = 0.) noexcept
    {
        modify([&,after,repeat](){ ev_timer_set(wptr(), after, repeat); });
        return *this;
    }

    void start(timestamp after, timestamp repeat = 0.) noexcept
    {
        modify([&,after,repeat](){ ev_timer_set(wptr(), after, repeat); });
        start();
    }

    void again() noexcept
    { async() << [&](evloop el) { ev_timer_again(el, wptr()); }; }
};

class periodic : public watcher<periodic, ev_periodic>
{
public:
    using base_type = watcher<periodic, ev_periodic>;
    using base_type::start;
    using base_type::stop;
    using base_type::operator=;

    periodic()
    : base_type(ev_periodic_start, ev_periodic_stop)
    {}

    periodic(periodic&& other)
    { *this = std::move(other); }

    periodic(const periodic& other) = delete;
    periodic& operator=(const periodic& other) = delete;

    periodic& operator=(periodic&& other)
    {
        base_type::operator=(std::forward<base_type>(other));

        return *this;
    }

    periodic& operator()(timestamp at, timestamp interval = 0.) noexcept
    {
        modify([&,at,interval](){ ev_periodic_set(wptr(), at, interval, 0); });
        return *this;
    }

    void start(timestamp at, timestamp interval = 0.) noexcept
    {
        modify([&,at,interval](){ ev_periodic_set(wptr(), at, interval, 0); });
        start();
    }

    void again() noexcept
    { async() << [&](evloop el) { ev_periodic_again(el, wptr()); }; }
};

class NX_API after
{
public:
    after(timestamp timeout);

    after& operator<<(void_cb&& cb);

private:
    timestamp timeout_;
};

} // namespace nx

#endif // __NX_WATCHERS_H__
