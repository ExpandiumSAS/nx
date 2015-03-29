#include <iostream>

#include <nx/loop.hpp>

namespace nx {

loop&
loop::get()
{
    static loop l;
    return l;
}

loop&
loop::operator()(loop_cb&& cb)
{ return enqueue(std::move(cb)); }

loop&
loop::operator()(void_cb&& cb)
{ return enqueue([cb = std::move(cb)](evloop l) { cb(); }); }

loop&
loop::operator<<(loop_cb&& cb)
{ return enqueue(std::move(cb)); }

loop&
loop::operator<<(void_cb&& cb)
{ return enqueue([cb = std::move(cb)](evloop l) { cb(); }); }

loop::loop()
: l_(ev_default_loop())
{
    ev_async_init(
        &async_w_,
        [](evloop el, ev_async* a, int events) {}
    );

    ev_async_start(l_, &async_w_);
    start();
}

loop::~loop()
{
    stop();
    ev_loop_destroy(l_);
}

void
loop::start()
{
    t_ = std::thread(
        [this]() {
            ev_loop_fork(l_);

            while (!stop_.load()) {
                loop_cb cb;

                while (q_.try_dequeue(cb)) {
                    cb(l_);
                }

                ev_run(l_, EVRUN_ONCE);
            }
        }
    );
}

void
loop::stop()
{
    stop_ = true;

    enqueue(
        [&](evloop l) {
            ev_break(l, EVBREAK_ALL);
            ev_async_stop(l, &async_w_);
        }
    );

    t_.join();
}

loop&
loop::enqueue(loop_cb&& cb)
{
    if (q_.enqueue(cb)) {
        ev_async_send(l_, &async_w_);
    } else {
        std::cerr << "failed to queue cb" << std::endl;
    }

    return *this;
}

loop&
async()
{ return loop::get(); }

} // namespace nx
