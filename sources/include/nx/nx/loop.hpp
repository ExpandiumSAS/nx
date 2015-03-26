#ifndef __NX_LOOP_H__
#define __NX_LOOP_H__

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

#include <nx/ev.hpp>
#include <nx/config.h>

namespace nx {

using evloop = struct ev_loop*;
using timestamp = ev_tstamp;

class NX_API loop
{
public:
    using loop_cb = std::function<void()>;

    static loop& get();

    void start();
    void stop();

    void operator()(loop_cb cb);

private:
    loop();
    ~loop();

    loop(const loop&) = delete;
    loop(loop&&) = delete;
    void operator=(const loop&) = delete;
    void operator=(loop&&) = delete;

    void lock();
    void unlock();

    std::recursive_mutex m_;
    std::condition_variable cv_;
    std::thread t_;
    struct ev_async async_w_;
};

} // namespace nx

#endif // __NX_LOOP_H__
