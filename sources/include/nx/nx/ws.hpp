#ifndef __NX_WS_H__
#define __NX_WS_H__

#include <nx/config.h>
#include <nx/tcp.hpp>
#include <nx/request.hpp>
#include <nx/reply.hpp>

namespace nx {

class NX_API ws : public tcp_base<ws>
{
public:
    using base_type = tcp_base<ws>;
    using this_type = ws;

    ws();
    ws(int fh);
    ws(const ws& other) = delete;
    ws(ws&& other);
    virtual ~ws();

    ws& operator=(ws&& other);

    void process_request();
    void process_reply();
    void send_request();

private:
    bool request_parsed();
    bool reply_parsed();

    std::string server_challenge() const;
    void server_handshake();
    void client_handshake();

    bool parsed_;
    request req_;
    reply rep_;
};

} // namespace nx

#endif // __NX_WS_H__
