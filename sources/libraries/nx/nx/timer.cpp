#include <nx/timer.hpp>
#include <nx/service.hpp>

namespace nx {

timer::timer()
: t_(service::get().io_service())
{}

timer&
timer::operator()(const timestamp& after)
{ t_.expires_from_now(after); }

timer::start()
{
    auto self(shared_from_this());

    t_.async_wait(
        [this,self](const error_code& ec) {
            if ()
        }
    );
}

timer::stop()
{}

timer&
timer::operator=(timer_cb cb)
{
    cb_ = cb;
    start();

    return *this;
}

} // nx
