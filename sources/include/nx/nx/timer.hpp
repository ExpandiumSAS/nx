#ifndef __NX_TIMER_H__
#define __NX_TIMER_H__

#include <functional>

#include <boost/asio.hpp>

#include <nx/config.h>
#include <nx/object.hpp>

namespace nx {

namespace asio = boost::asio;

using timestamp = asio::deadline_timer::duration_type;

class NX_API timer : public object;
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

    void start();
    void stop();

    timer& operator=(timer_cb cb);

private:
    asio::deadline_timer t_;
    timer_cb cb_;
};

} // nx


#endif // __NX_TIMER_H__
