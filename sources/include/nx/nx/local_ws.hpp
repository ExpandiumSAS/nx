#ifndef __NX_LOCAL_WS_H__
#define __NX_LOCAL_WS_H__

#include <stdint.h>

#include <nx/config.h>
#include <nx/local_socket.hpp>
#include <nx/request.hpp>
#include <nx/reply.hpp>
#include <nx/buffer.hpp>
#include <nx/context.hpp>
#include <nx/ws.hpp>

namespace nx {

class NX_API local_ws
    : public local_socket_base<local_ws>
{
  public:
    using base_type = local_socket_base<local_ws>;
    using this_type = local_ws;

    local_ws()
    : uid_(make_uid())
    {}

    template <typename OtherSocket>
    local_ws(OtherSocket &&sock)
    : base_type(std::move(sock)),
    uid_(make_uid())
    {}

    virtual void start();

    bool parse_frame(ws_frame &f);
    void process_frames();
    void process_close();
    void send_request();

    void set_callbacks(const ws_connection& w);

    std::string uid();
    const std::string& uid() const;

    static void server_handshake(const request &req, reply &rep);
    static std::string server_challenge(const request &req);

private:
    void finish(uint16_t code);

    void client_handshake();

    void send_close_frame(uint16_t code);
    void send_ping_pong_frame(bool ping);

    ws_ptr self();

    static std::string make_uid();

private:
    bool parsed_ = false;
    std::string uid_;


    ws_connect_cb connect_cb_;
    ws_message_cb message_cb_;
    ws_finish_cb finish_cb_;
};

} // namespace nx

#endif // __NX_LOCAL_WS_H__