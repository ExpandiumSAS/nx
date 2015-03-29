#ifndef __NX_LOOP_H__
#define __NX_LOOP_H__

#include <thread>
#include <atomic>
#include <functional>

#include <moodycamel/concurrentqueue.hpp>

#include <nx/ev.hpp>
#include <nx/config.h>

namespace nx {

using evloop = struct ev_loop*;
using timestamp = ev_tstamp;

using loop_cb = std::function<void(evloop)>;
using void_cb = std::function<void()>;

class NX_API loop
{
public:
    static loop& get();

    loop& operator()(loop_cb&& cb);
    loop& operator()(void_cb&& cb);
    loop& operator<<(loop_cb&& cb);
    loop& operator<<(void_cb&& cb);

private:
    loop();
    ~loop();

    loop(const loop&) = delete;
    loop(loop&&) = delete;
    void operator=(const loop&) = delete;
    void operator=(loop&&) = delete;

    void start();
    void stop();

    loop& enqueue(loop_cb&& cb);

private:
    using queue_type = moodycamel::ConcurrentQueue<loop_cb>;

    evloop l_;
    struct ev_async async_w_;
    std::atomic_bool stop_{false};
    std::thread t_;
    queue_type q_;
};

NX_API
loop&
async();

} // namespace nx

#endif // __NX_LOOP_H__
