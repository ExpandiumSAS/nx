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

class NX_API client
{
public:
    using reply_cb = http::reply_cb;
    using done_cb = std::function<void()>;

    client(const method& m, const endpoint& ep, done_cb cb);
    client(const client& other) = delete;
    client(client&& other);
    virtual ~client();

    client& operator=(const client& other) = delete;
    client& operator=(client&& other);

    client& operator/(const std::string& path);

    template <typename T>
    client& operator<<(const T& t)
    {
        req_ << t;
        return *this;
    }

    client& operator=(reply_cb cb);

private:
    void start();

    http http_;
    request req_;
    endpoint ep_;
    reply_cb reply_cb_;
    done_cb done_cb_;
};

using client_map = std::unordered_map<std::size_t, client>;

class NX_API httpc
{
public:
    httpc();
    httpc(const httpc& other) = delete;
    httpc(httpc&& other);
    virtual ~httpc();

    httpc& operator=(httpc&& other);

    client& operator()(const method& m, const endpoint& ep);

private:
    std::size_t next_id_;
    client_map clients_;
};

} // namespace nx

#endif // __NX_HTTPC_H__
