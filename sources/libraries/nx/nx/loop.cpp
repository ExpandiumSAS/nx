#include <ev.h>

#include <nx/loop.hpp>

namespace nx {

void
loop::start()
{
    static loop l;

    ev_set_userdata(ev_default_loop(), static_cast<void*>(&l));
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

void
loop::lock()
{ m_.lock(); }

void
loop::unlock()
{ m_.unlock(); }

loop::loop()
{}

loop::~loop()
{}

void
loop::run()
{
    t_ = std::thread(
        [this]() {
            lock();
            ev_run(0);
            unlock();
        }
    );
}

} // namespace nx
