#include <memory>

#include <nx/service.hpp>

namespace nx {

std::once_flag flag;
using service_ptr = std::unique_ptr<service>;

service_ptr _sptr_;

service&
service::get()
{
    std::call_once(flag, [](){ _sptr_ = std::make_unique<service>(); });

    return *_sptr_;
}

service::service()
: service_(),
work_(service_),
t_()
{ start(); }

void
service::start()
{
    if (t_.joinable()) {
        // Already running
        return;
    }

    t_ = std::thread([this]() { service_.run(); });
}

void
service::stop()
{
    if (!t_.joinable()) {
        // Not running
        return;
    }

    service_.stop();
    t_.join();
    service_.reset();
}

} // namespace nx
