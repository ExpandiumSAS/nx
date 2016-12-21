#ifndef __NX_HTTP_H__
#define __NX_HTTP_H__

#include <functional>

#include <nx/config.h>
#include <nx/tcp.hpp>
#include <nx/request.hpp>
#include <nx/reply.hpp>
#include <nx/handlers.hpp>
#include <nx/cond_var.hpp>
#include <nx/ws.hpp>

namespace nx {

using request_cb = std::function<
    void(request& req, buffer& data, reply& rep)
>;
using reply_cb = std::function<
    void(reply& rep, buffer& data)
>;

struct http_async_tag {};
struct http_sync_tag {};

class NX_API http : public tcp_base<http>
{
public:
    using base_type = tcp_base<http>;
    using this_type = http;

    using base_type::tcp_base;

    http() = default;
    http(request&& req, reply_cb&& cb);
    http(request&& req, reply_cb&& cb, asio::io_service& io);
    http(reply&& rep, request_cb&& cb);
    http(const http& other) = delete;
    http(http&& other) = default;
    http& operator=(const http& other) = delete;
    http& operator=(http&& other) = default;

    void process_request();
    bool process_reply();
    void send_request();

    using base_type::operator<<;

    http& operator<<(request_cb cb);
    http& operator<<(reply_cb cb);

    template<
        typename SocketType,
        typename ...Args
    >
    SocketType& upgrade_connection(Args&& ...args)
    {
        auto upg_ct = new_object<SocketType>(sock(), std::forward<Args>(args)...);
        auto& upg = *upg_ct;

        return upg;
    }

private:
    bool request_parsed();
    bool reply_parsed();
    void call_or_fail(void_cb cb);

    void process_upgrade();

    bool parsed_ = false;
    request req_;
    reply rep_;
    request_cb request_cb_;
    reply_cb reply_cb_;
};

template <typename OnRequest>
endpoint
serve(http& h, const endpoint& ep, OnRequest&& cb)
{
    return
        serve(
            h,
            ep,
            [cb = std::move(cb)](http& c) {
                c << std::move(cb);
            },
            [](http& c) {
                c.process_request();
            }
        );
}


template <typename OnReply>
http&
async_connect(const endpoint& ep, request&& req, OnReply&& cb)
{
    auto p = new_object<http>(std::move(req), std::move(cb));
    auto& h = *p;

    h[tags::on_read] = [](http& t) {
        t.process_reply();
    };

    return connect(
        h,
        ep,
        [](http& t) {
            t.send_request();
        }
    );
}

template <typename OnReply>
http&
sync_connect(const endpoint& ep, request&& req, OnReply&& cb)
{
    auto t = std::make_shared<task>();
    auto p = new_object<http>(std::move(req), std::move(cb), t->get_io_service());

    auto& h = *p;
    cond_var cv;

    h[tags::on_read] = [&cv](http& t) {
        if (t.process_reply()) {
            cv.notify();    
        }
    };

    auto& result = connect(
        h,
        ep,
        [](http& t) {
            t.send_request();
        }
    );

    cv.wait();
    t->stop();

    return result;
}

} // namespace nx

#endif // __NX_HTTP_H__
