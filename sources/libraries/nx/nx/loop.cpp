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
            ev_loop_fork(l_);

            lock();
            ev_run(l_, 0);
            unlock();
        }
    );
}

void
loop::stop()
{
    {
        std::lock_guard<std::recursive_mutex> lock(m_);
        ev_break(l_, EVBREAK_ALL);
        ev_async_send(l_, &async_w_);
        ev_async_stop(l_, &async_w_);
    }

    t_.join();
}

void
loop::operator()(loop_cb cb)
{
    std::lock_guard<std::recursive_mutex> lock(m_);
    cb(l_);
    ev_async_send(l_, &async_w_);
}

void
loop::lock()
{ m_.lock(); }

void
loop::unlock()
{ m_.unlock(); }

loop::loop()
: l_(ev_default_loop())
{
    ev_async_init(
        &async_w_,
        [](evloop el, ev_async* a, int events) {}
    );

    ev_async_start(l_, &async_w_);

    ev_set_userdata(l_, static_cast<void*>(this));

    ev_set_loop_release_cb(
        l_,
        [](evloop el) {
            auto& l = *static_cast<loop*>(ev_userdata(el));
            l.unlock();
        },
        [](evloop el) {
            auto& l = *static_cast<loop*>(ev_userdata(el));
            l.lock();
        }
    );
}

loop::~loop()
{
    stop();
    ev_loop_destroy(l_);
}

} // namespace nx
