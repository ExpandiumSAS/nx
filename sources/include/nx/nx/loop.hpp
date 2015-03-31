#ifndef __NX_LOOP_H__
#define __NX_LOOP_H__

#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <unordered_set>

#include <moodycamel/concurrentqueue.hpp>

#include <nx/config.h>
#include <nx/ev.hpp>
#include <nx/handle_base.hpp>

namespace nx {

using evloop = struct ev_loop*;
using timestamp = ev_tstamp;

using loop_cb = std::function<void(evloop)>;
using void_cb = std::function<void()>;

struct loop_op
{
    loop_cb cb;
    void_cb h;
};

class NX_API loop
{
public:
    static loop& get();

    loop& operator<<(loop_cb&& cb);
    loop& operator<<(void_cb&& cb);
    loop& operator<<(loop_op&& op);

    void add_handle(handle_ptr p);
    void remove_handle(handle_ptr p);

private:
    loop();
    ~loop();

    loop(const loop&) = delete;
    loop(loop&&) = delete;
    void operator=(const loop&) = delete;
    void operator=(loop&&) = delete;

    void start();
    void stop();

    loop& enqueue(loop_op&& op);

private:
    using queue_type = moodycamel::ConcurrentQueue<loop_op>;

    evloop l_;
    struct ev_async async_w_;
    std::atomic_bool stop_{false};
    std::thread t_;
    queue_type q_;
    handle_set handles_;
    std::mutex hm_;
};

NX_API
loop&
async();

template <typename Handle, typename... Args>
std::shared_ptr<Handle>
new_handle(Args&&... args)
{
    auto p = std::make_shared<Handle>(std::forward<Args>(args)...);

    loop::get().add_handle(handle_ptr(p));

    return p;
}

} // namespace nx

#endif // __NX_LOOP_H__
