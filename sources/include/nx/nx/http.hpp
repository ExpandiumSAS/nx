#ifndef __NX_HTTP_H__
#define __NX_HTTP_H__

#include <functional>

#include <nx/config.h>
#include <nx/tcp.hpp>
#include <nx/request.hpp>
#include <nx/reply.hpp>

namespace nx {

using request_cb = std::function<
    void(request& req, buffer& data, reply& rep)
>;
using reply_cb = std::function<
    void(reply& rep, buffer& data)
>;

class NX_API http : public tcp_base<http>
{
public:
    using base_type = tcp_base<http>;
    using this_type = http;

    using base_type::tcp_base;

    http() = default;
    http(const http& other) = delete;
    http(http&& other) = default;
    http& operator=(const http& other) = delete;
    http& operator=(http&& other) = default;

    void process_request();
    void process_reply();
    void send_request();

    using base_type::operator<<;

    http& operator<<(request_cb cb);
    http& operator<<(request req);
    http& operator<<(reply_cb cb);
    http& operator<<(reply rep);

private:
    bool request_parsed();
    bool reply_parsed();

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
connect(const endpoint& ep, request req, OnReply&& cb)
{
    auto p = new_object<http>();
    auto& h = *p;

    h
        << std::move(req)
        << std::move(cb)
        ;

    h[tags::on_read] = [](http& t) {
        t.process_reply();
    };

    return
        connect(
            h,
            ep,
            [](http& t) {
                t.send_request();
            }
        );
}

} // namespace nx

#endif // __NX_HTTP_H__
