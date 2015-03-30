#ifndef __NX_HTTP_H__
#define __NX_HTTP_H__

#include <nx/config.h>
#include <nx/tcp.hpp>
#include <nx/request.hpp>
#include <nx/reply.hpp>

namespace nx {

class NX_API http : public tcp_base<http>
{
public:
    using base_type = tcp_base<http>;
    using this_type = http;
    using request_cb = std::function<
        void(request& req, buffer& data, reply& rep)
    >;
    using reply_cb = std::function<
        void(reply& rep, buffer& data)
    >;

    http();
    http(const http& other) = delete;
    http(http&& other);
    virtual ~http();

    http& operator=(http&& other);

    void process_request();
    void process_reply();
    void send_request();

    const endpoint& operator()(
        const endpoint& ep,
        const std::string& root,
        request_cb cb
    );

    void operator()(
        const endpoint& ep,
        request&& req,
        reply_cb cb
    );

private:
    bool request_parsed();
    bool reply_parsed();

    bool parsed_;
    request req_;
    reply rep_;
    request_cb request_cb_;
    reply_cb reply_cb_;
};

} // namespace nx

#endif // __NX_HTTP_H__
