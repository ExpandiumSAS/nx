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

class NX_API httpc
{
public:
    using done_cb = std::function<void()>;

    httpc(const method& m, const endpoint& ep);
    httpc(const httpc& other) = delete;
    httpc(httpc&& other);
    virtual ~httpc();

    httpc& operator=(const httpc& other) = delete;
    httpc& operator=(httpc&& other);

    httpc& operator/(const std::string& path);

    template <typename T>
    httpc& operator<<(const T& t)
    {
        req_ << t;
        return *this;
    }

    httpc& operator=(reply_cb cb);

private:
    void start();

    request req_;
    endpoint ep_;
    reply_cb reply_cb_;
};

} // namespace nx

#endif // __NX_HTTPC_H__
