#ifndef __NX_WS_H__
#define __NX_WS_H__

#include <stdint.h>

#include <nx/config.h>
#include <nx/tcp.hpp>
#include <nx/request.hpp>
#include <nx/reply.hpp>

namespace nx {

struct NX_API ws_frame
{
    bool fin;
    bool rsv1;
    bool rsv2;
    bool rsv3;
    uint8_t opcode;
    buffer payload;
};

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

    bool parse_frame(ws_frame& f);
    void process_frames();
    void process_request();
    void process_reply();
    void send_request();

private:
    void finish(int code);
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
