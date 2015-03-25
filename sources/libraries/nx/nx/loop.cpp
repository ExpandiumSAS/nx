#include <ev.h>

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
            lock();
            ev_run(ev_default_loop(), 0);
            unlock();
        }
    );
}

void
loop::stop()
{
    ev_break(ev_default_loop(), 0);
    t_.join();
}

void
loop::lock()
{ m_.lock(); }

void
loop::unlock()
{ m_.unlock(); }

loop::loop()
{
    ev_set_userdata(ev_default_loop(), static_cast<void*>(this));
    ev_set_loop_release_cb(
        ev_default_loop(),
        [](struct ev_loop* el) {
            auto& l = *static_cast<loop*>(ev_userdata(el));
            l.unlock();
        },
        [](struct ev_loop* el) {
            auto& l = *static_cast<loop*>(ev_userdata(el));
            l.lock();
        }
    );
}

loop::~loop()
{}

} // namespace nx
