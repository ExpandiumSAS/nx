#include <iostream>

#include <nx/loop.hpp>

namespace nx {

loop&
loop::get()
{
    static loop l;
    return l;
}

void
loop::start()
{
    t_ = std::thread(
        [this]() {
            ev_loop_fork();

            lock();
            ev_run(0);
            unlock();
        }
    );
}

void
loop::stop()
{
    {
        std::lock_guard<std::recursive_mutex> lock(m_);
        ev_break(EVBREAK_ALL);
        ev_async_send(&async_w_);
        ev_async_stop(&async_w_);
    }

    t_.join();
}

void
loop::operator()(loop_cb cb)
{
    std::lock_guard<std::recursive_mutex> lock(m_);
    cb();
    ev_async_send(&async_w_);
}

void
loop::lock()
{ m_.lock(); }

void
loop::unlock()
{ m_.unlock(); }

loop::loop()
{
    ev_async_init(
        &async_w_,
        [](ev_async* a, int events) {}
    );

    ev_async_start(&async_w_);

    ev_set_userdata(static_cast<void*>(this));

    ev_set_loop_release_cb(
        []() {
            auto& l = *static_cast<loop*>(ev_userdata());
            l.unlock();
        },
        []() {
            auto& l = *static_cast<loop*>(ev_userdata());
            l.lock();
        }
    );
}

loop::~loop()
{ stop(); }

} // namespace nx
