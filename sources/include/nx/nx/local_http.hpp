#ifndef __NX_LOCAL_HTTP_H__
#define __NX_LOCAL_HTTP_H__

#include <functional>

#include <nx/config.h>
#include <nx/local_socket.hpp>
#include <nx/request.hpp>
#include <nx/reply.hpp>
#include <nx/handlers.hpp>
#include <nx/cond_var.hpp>
#include <nx/http.hpp>

namespace nx {

/*struct http_async_tag {};
struct http_sync_tag {};*/

class NX_API local_http : public local_socket_base<local_http>
{
public:
    using base_type = local_socket_base<local_http>;
    using this_type = local_http;

    using base_type::local_socket_base;

    local_http() = default;
    local_http(request&& req, reply_cb&& cb);
    local_http(request&& req, reply_cb&& cb, asio::io_service& io);
    local_http(reply&& rep, request_cb&& cb);
    local_http(const local_http& other) = delete;
    local_http(local_http&& other) = default;
    local_http& operator=(const local_http& other) = delete;
    local_http& operator=(local_http&& other) = default;

    void process_request();
    bool process_reply();
    void send_request();

    using base_type::operator<<;

    local_http& operator<<(request_cb cb);
    local_http& operator<<(reply_cb cb);

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
endpoint_local
serve(local_http& h, const endpoint_local& ep, OnRequest&& cb)
{
    return
        serve(
            h,
            ep,
            [cb = std::move(cb)](local_http& c) {
                c << std::move(cb);
            },
            [](local_http& c) {
                c.process_request();
            }
        );
}


template <typename OnReply>
local_http&
async_connect(const endpoint_local& ep, request&& req, OnReply&& cb)
{
    auto p = new_object<local_http>(std::move(req), std::move(cb));
    auto& h = *p;

    h[tags::on_read] = [](local_http& t) {
        t.process_reply();
    };

    return connect(
        h,
        ep,
        [](local_http& t) {
            t.send_request();
        }
    );
}

template <typename OnReply>
local_http&
sync_connect(const endpoint_local& ep, request&& req, OnReply&& cb, int32_t timeout_s)
{
    auto t = std::make_shared<task>();
    auto p = new_object<local_http>(std::move(req), std::move(cb), t->get_io_service());

    auto& h = *p;
    cond_var cv;

    h[tags::on_read] = [&cv](local_http& t) {
        if (t.process_reply()) {
            cv.notify();
        }
    };

    auto& result = connect(
        h,
        ep,
        [](local_http& t) {
            t.send_request();
        }
    );

    if (timeout_s > 0) {
        cv.wait_for((uint64_t)timeout_s * 1000);
    } else {
        cv.wait();
    }
    t->stop();

    return result;
}

} // namespace nx

#endif // __NX_LOCAL_HTTP_H__
