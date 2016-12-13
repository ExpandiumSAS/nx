#pragma once

#include <functional>

#include <nx/config.h>
#include <nx/buffer.hpp>
#include <string>

namespace nx {

/// Forwzard declaration
class ws;

/// WS contextual class
struct context {
    context(ws& w)
    : w_(w)
    {}

    context& operator<< (const buffer& data);
    context& operator<< (const std::string& text);

    ws& w_;
};

/// WS Connection Callback 
using ws_connect_cb = std::function<
    void(context&)
>;

/// WS Message Callback
using ws_message_cb = std::function<
    void(context&, buffer& data)
>;

/// WS Finish Callback
using ws_finish_cb = std::function<
    void(context&)
>;

struct ws_tag {};
const ws_tag WS;

struct ws_connection {
    ws_connection() = default;
    ws_connection(const ws_connection& ) = default;
    ws_connection(ws_connection&& ) = default;

    ws_connection& operator=(const ws_connection& ) = default;
    ws_connection& operator=(ws_connection&& ) = default;
    
    ws_connection(ws_connect_cb ccb, ws_message_cb mcb, ws_finish_cb fcb)
    : connect_cb(ccb),
      message_cb(mcb),
      finish_cb(fcb)
    {}

    ws_connection(ws_message_cb mcb)
    : message_cb(mcb)
    {}

    ws_connect_cb   connect_cb;
    ws_message_cb   message_cb;
    ws_finish_cb    finish_cb;
};

NX_API void encode_frame_data(buffer& b, bool binary, const buffer& data);

}   // namespace nx