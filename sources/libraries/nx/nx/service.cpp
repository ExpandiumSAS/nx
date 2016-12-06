#include <memory>

#include <nx/service.hpp>
#include <nx/handle_error.hpp>

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

service&
service::operator<<(void_cb&& cb)
{
    io_service_.post(
        [cb = std::move(cb)]() {
            cb();
        }
    );

    return *this;
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

            {
                std::lock_guard<std::mutex> lock(tasks_mutex_);

                for (auto& t : available_tasks_) {
                    t->stop();
                }

                for (auto& t : runnable_tasks_) {
                    t->stop();
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
                available_tasks_.clear();
                runnable_tasks_.clear();
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

task_ptr 
service::available_task()
{
    std::lock_guard<std::mutex> lock(tasks_mutex_);

    task_ptr result;
    if (available_tasks_.empty()) {
        result = std::make_shared<task>();
        runnable_tasks_.insert(result->ptr());
    } else {
        result = std::dynamic_pointer_cast<task>(*(available_tasks_.begin()));
        available_tasks_.erase(result);
        runnable_tasks_.insert(result);
    }
    return result;
}

void
service::remove_task(task_ptr p)
{
    std::lock_guard<std::mutex> lock(tasks_mutex_);

    available_tasks_.insert(p);
    runnable_tasks_.erase(p);
}

void
add_object(object_ptr ptr)
{ service::get().add(ptr); }

void
remove_object(object_ptr ptr)
{ service::get().remove(ptr); }

void
stop()
{ service::get().stop(); }

service&
async()
{ return service::get(); }


} // namespace nx
