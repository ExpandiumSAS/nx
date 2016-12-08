#ifndef __NX_WS_H__
#define __NX_WS_H__

#include <stdint.h>

#include <nx/config.h>
#include <nx/tcp.hpp>
#include <nx/request.hpp>
#include <nx/reply.hpp>
#include <nx/buffer.hpp>
#include <nx/context.hpp>

namespace nx {

using ws_connect_cb = std::function<
    void(context&)
>;
using ws_message_cb = std::function<
    void(context&, buffer& data)
>;
using ws_finish_cb = std::function<
    void(context&)
>;

struct NX_API ws_frame
{
    bool fin;
    bool rsv1;
    bool rsv2;
    bool rsv3;
    uint8_t opcode;
    buffer payload;
};

const uint8_t WS_OP_CONTINUATION_FRAME = 0x0;
const uint8_t WS_OP_TEXT_FRAME = 0x1;
const uint8_t WS_OP_BINARY_FRAME = 0x2;
const uint8_t WS_OP_CLOSE = 0x8;
const uint8_t WS_OP_PING = 0x9;
const uint8_t WS_OP_PONG = 0xA;  

class NX_API ws 
: public tcp_base<ws>
{
public:
    using base_type = tcp_base<ws>;
    using this_type = ws;

    ws()
    : ctx_{*this}
    {}

    bool parse_frame(ws_frame& f);
    void process_frames();
    void process_request();
    void process_reply();
    void send_request();

private:
    void finish(uint16_t code);
    bool request_parsed();
    bool reply_parsed();

    std::string server_challenge() const;
    void server_handshake();
    void client_handshake();

    void send_close_frame(uint16_t code);
    void send_ping_pong_frame(bool ping);

    bool parsed_ = false;
    request req_;
    reply rep_;
    context ctx_;

    ws_connect_cb   connect_cb_;
    ws_message_cb   message_cb_;
    ws_finish_cb    finish_cb_;

};

} // namespace nx

#endif // __NX_WS_H__
