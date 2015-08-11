#ifndef __NX_SERVICE_H__
#define __NX_SERVICE_H__

#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include <boost/asio.hpp>

#include <nx/config.h>
#include <nx/object.hpp>

namespace nx {

namespace asio = boost::asio;

class NX_API service
{
public:
    static service& get();

    service();
    ~service();

    void start();
    void stop();

    asio::io_service& io_service();
    const asio::io_service& io_service() const;

    void add(object_ptr sptr);
    void remove(object_ptr sptr);

private:


    service(const service&) = delete;
    void operator=(const service&) = delete;

    asio::io_service io_service_;
    asio::io_service::work work_;
    std::thread t_;

    std::unordered_set<object_ptr> objects_;
    std::mutex objects_mutex_;
};

} // namespace nx

#endif // __NX_SERVICE_H__
