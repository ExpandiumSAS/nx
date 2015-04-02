#ifndef __NX_LOOP_H__
#define __NX_LOOP_H__

#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <unordered_set>

#include <moodycamel/concurrent_queue.hpp>

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

    void start();
    void stop();

    loop& operator<<(loop_cb&& cb);
    loop& operator<<(void_cb&& cb);
    loop& operator<<(loop_op&& op);

    void register_handle(handle_ptr p);
    void unregister_handle(handle_ptr p);

private:
    loop();
    ~loop();

    loop(const loop&) = delete;
    loop(loop&&) = delete;
    void operator=(const loop&) = delete;
    void operator=(loop&&) = delete;

    bool run(int flags);

    loop& enqueue(loop_op&& op);

private:
    using queue_type = moodycamel::concurrent_queue<loop_op>;

    evloop l_;
    struct ev_async async_w_;
    std::atomic_bool stop_{false};
    std::thread t_;
    queue_type q_;
    handle_set handles_;
    std::mutex hm_;
};

NX_API
void
stop();

NX_API
loop&
async();

NX_API
void
register_handle(handle_ptr p);

NX_API
void
unregister_handle(handle_ptr p);

template <typename Handle, typename... Args>
std::shared_ptr<Handle>
new_handle(Args&&... args)
{
    auto p = std::make_shared<Handle>(std::forward<Args>(args)...);

    register_handle(p);

    return p;
}

} // namespace nx

#endif // __NX_LOOP_H__
