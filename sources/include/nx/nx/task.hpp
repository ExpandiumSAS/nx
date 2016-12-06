#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include <boost/asio.hpp>

#include <nx/config.h>
#include <nx/object_base.hpp>
#include <nx/handlers.hpp>

namespace nx {

namespace asio = boost::asio;

class NX_API task
: public object_base
{
public:
    task()
    : io_service_(),
      work_(io_service_),
      t_([this](){ 
          io_service_.run();
          io_service_.reset(); 
      })
    {}

    task(const task& ) = delete;
    task& operator= (const task& ) = delete;

    asio::io_service& get_io_service()
    { return io_service_; }

    const asio::io_service& get_io_service() const
    { return io_service_; }

    virtual void stop() override
    {
        io_service_.stop();
        t_.join();
        io_service_.reset();
    }

private:
    asio::io_service io_service_;
    asio::io_service::work work_;
    std::thread t_;
};

using task_ptr = std::shared_ptr<task>;

}   // namespace nx