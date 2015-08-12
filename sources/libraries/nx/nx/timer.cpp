#include <nx/timer.hpp>
#include <nx/service.hpp>

namespace nx {

timer::timer()
: t_(service::get().io_service())
{}

timer&
timer::operator()(const timestamp& after)
{
    t_.expires_from_now(after);

    return *this;
}

timer&
timer::operator()(std::size_t seconds)
{ return (*this)(std::chrono::seconds(seconds)); }

void
timer::start()
{
    t_.async_wait(
        [this](const error_code& ec) {
            if (handle_error(*this, "timer wait", ec)) {
                return;
            }

            cb_(*this);
        }
    );
}

void
timer::stop()
{
    error_code ec;

    t_.cancel(ec);
    handle_error(*this, "timer cancel", ec);
}

timer&
timer::operator=(timer_cb cb)
{
    cb_ = cb;

    return *this;
}

} // nx
