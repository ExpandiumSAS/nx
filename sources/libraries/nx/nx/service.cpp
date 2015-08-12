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
: io_service_(),
work_(io_service_),
t_()
{ start(); }

service::~service()
{ stop(); }

asio::io_service&
service::io_service()
{ return io_service_; }

const asio::io_service&
service::io_service() const
{ return io_service_; }

void
service::add(object_ptr sptr)
{
    std::lock_guard<std::mutex> lock(objects_mutex_);

    objects_.insert(sptr);
}

void
service::remove(object_ptr sptr)
{
    std::lock_guard<std::mutex> lock(objects_mutex_);

    objects_.erase(sptr);
}

void
service::start()
{
    if (t_.joinable()) {
        // Already running
        return;
    }

    t_ = std::thread([this]() { io_service_.run(); });
}

void
service::stop()
{
    if (!t_.joinable()) {
        // Not running
        return;
    }

    io_service_.stop();
    t_.join();
    io_service_.reset();
}

void
stop()
{ service::get().stop(); }


} // namespace nx
