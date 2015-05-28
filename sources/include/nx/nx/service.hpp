#ifndef __NX_SERVICE_H__
#define __NX_SERVICE_H__

#include <boost/asio.hpp>

#include <thread>
#include <unordered_map>

#include <nx/config.h>

namespace nx {

using namespace asio = boost::asio;

class NX_API service
{
public:
    static service& get();

    service();
    ~service();

    void start();
    void stop();

    void serve(const std::string& )

private:
    asio::service service_;
    asio::io_service::work work_;
    std::thread t_;

    std::unordered_map<std::string, asio::ip::tcp::acceptor> acceptors_;
};



} // namespace nx

#endif // __NX_SERVICE_H__
