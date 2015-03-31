#ifndef __NX_HTTPC_H__
#define __NX_HTTPC_H__

#include <unordered_map>
#include <functional>

#include <nx/config.h>
#include <nx/http.hpp>
#include <nx/methods.hpp>
#include <nx/request.hpp>
#include <nx/reply.hpp>

namespace nx {

class NX_API http_request
{
public:
    http_request(const method& m, const endpoint& ep);
    http_request(const http_request& other) = delete;
    http_request(http_request&& other);
    virtual ~http_request();

    http_request& operator=(const http_request& other) = delete;
    http_request& operator=(http_request&& other);

    http_request& operator/(const std::string& path);

    template <typename T>
    http_request& operator<<(const T& t)
    {
        req_ << t;
        return *this;
    }

    http_request& operator=(reply_cb cb);

private:
    void start();

    request req_;
    endpoint ep_;
    reply_cb reply_cb_;
};

class NX_API httpc
{
public:
    http_request operator()(const method& m, const endpoint& ep);
};

} // namespace nx

#endif // __NX_HTTPC_H__
