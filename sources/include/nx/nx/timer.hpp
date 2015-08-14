#ifndef __NX_TIMER_H__
#define __NX_TIMER_H__

#include <functional>

#include <boost/asio/system_timer.hpp>

#include <nx/config.h>
#include <nx/object.hpp>

namespace nx {

namespace asio = boost::asio;

using timestamp = asio::system_timer::duration;

class NX_API timer : public object<timer>
{
public:
    using timer_cb = std::function<
        void(timer& t)
    >;

    timer();

    timer(const timer& other) = delete;
    timer(timer&& other) = default;
    timer& operator=(const timer& other) = delete;
    timer& operator=(timer&& other) = default;

    timer& operator()(const timestamp& after);
    timer& operator()(std::size_t seconds);

    void start();
    void stop();

    timer& operator=(timer_cb cb);

private:
    asio::system_timer t_;
    timer_cb cb_;
};

class NX_API after
{
public:
    after(const timestamp& after);
    after(std::size_t seconds);

    void operator<<(void_cb&& cb);

private:
    timestamp timeout_;
};

} // nx


#endif // __NX_TIMER_H__
