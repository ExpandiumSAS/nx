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

    t_ = std::thread(
        [this]() {
            io_service_.run();
            io_service_.reset();

            {
                std::lock_guard<std::mutex> lock(objects_mutex_);

                for (auto& o : objects_) {
                    o->stop();
                }
            }

            std::size_t count = 0;
            const std::size_t max_count = 1000;
            error_code ec;

            while (io_service_.poll(ec) != 0) {
                if (ec) {
                    std::cout
                        << "service poll error: "
                        << ec.message()
                        << std::endl;
                }

                if (++count == max_count) {
                    std::cout
                        << "WHOA THERE ! "
                        << "there are still active objects after "
                        << max_count << " polls"
                        << std::endl;
                }
            }

            {
                std::lock_guard<std::mutex> lock(objects_mutex_);

                objects_.clear();
            }
        }
    );
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
