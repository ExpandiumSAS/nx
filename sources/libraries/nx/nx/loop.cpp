#include <iostream>
#include <sstream>
#include <stdexcept>

#include <nx/loop.hpp>

namespace nx {

loop&
loop::get()
{
    static loop l;
    return l;
}

loop&
loop::operator<<(loop_cb&& cb)
{ return enqueue({ std::move(cb), nullptr }); }

loop&
loop::operator<<(void_cb&& cb)
{ return enqueue({ nullptr, std::move(cb) }); }

loop&
loop::operator<<(loop_op&& op)
{ return enqueue(std::move(op)); }

void
loop::register_handle(handle_ptr p)
{
    std::lock_guard<std::mutex> g(hm_);

    handles_.insert(p);
}

void
loop::unregister_handle(handle_ptr p)
{
    std::lock_guard<std::mutex> g(hm_);

    handles_.erase(p);
}

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

bool
loop::run(int flags)
{
    loop_op op;

    while (q_.try_dequeue(op)) {
        if (op.cb) {
            op.cb(l_);
        }

        if (op.h) {
            op.h();
        }
    }

    if ((flags & EVRUN_NOWAIT) && ev_pending_count(l_) == 0) {
        return false;
    }

    return ev_run(l_, flags | EVFLAG_FORKCHECK);
}

void
loop::start()
{
    t_ = std::thread(
        [this]() {
            while (!stop_) {
                run(EVRUN_ONCE);
            }

            {
                std::lock_guard<std::mutex> g(hm_);

                for (auto& h : handles_) {
                    h->push_close();
                }
            }

            std::size_t count = 0;
            const std::size_t max_loop_count = 1000;

            while (run(EVRUN_NOWAIT)) {
                count++;

                if (count == max_loop_count) {
                    std::ostringstream oss;

                    oss
                        << "WHOA THERE !\n"
                        << "the loop couldn't stop after "
                        << max_loop_count << " iterations";

                    throw std::runtime_error(oss.str());
                }
            }
        }
    );
}

void
loop::stop()
{
    if (stop_) {
        return;
    }

    stop_ = true;

    *this << [&](evloop l) {
        ev_break(l, EVBREAK_ALL);
        ev_async_stop(l, &async_w_);
    };

    t_.join();
}

loop&
loop::enqueue(loop_op&& op)
{
    if (q_.enqueue(op)) {
        ev_async_send(l_, &async_w_);
    } else {
        std::cerr << "failed to queue loop operation" << std::endl;
    }

    return *this;
}

void
stop()
{ loop::get().stop(); }

loop&
async()
{ return loop::get(); }

void
register_handle(handle_ptr p)
{ loop::get().register_handle(p); }

void
unregister_handle(handle_ptr p)
{ loop::get().unregister_handle(p); }

} // namespace nx
