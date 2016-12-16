#pragma once

#include <functional>

#include <nx/config.h>
#include <nx/buffer.hpp>
#include <string>
#include <memory>

namespace nx {

/// Forwzard declaration
class ws; 
using ws_ptr = std::shared_ptr<ws>;
using ws_weak_ptr = std::weak_ptr<ws>;

const uint8_t text_frame_type = 0;
const uint8_t binary_frame_type = 1;

struct frame_type {
    const uint8_t value;
};

const frame_type ws_text = { text_frame_type };
const frame_type ws_binary = { binary_frame_type };
const frame_type ws_json = { text_frame_type };

/// WS contextual class
class NX_API context {
public:
    context() = default;
    context(ws_ptr w)
    : w_{w}
    {}

    ~context();

    context(const context& ) = delete;
    context(context&& ) = default;

    context& operator=(const context& ) = delete;
    context& operator=(context&& ) = default;

    context& operator<< (const frame_type& );
    context& operator<< (const buffer& data);
    context& operator<< (const std::string& text);

    void stop();

private:
    friend class ws;
    void done();
    
private:
    ws_weak_ptr w_;
    buffer data_;
    uint8_t type_{ text_frame_type };
};

/// WS Connection Callback 
using ws_connect_cb = std::function<
    void(context&&)
>;

/// WS Message Callback
using ws_message_cb = std::function<
    void(context&&, const buffer& data)
>;

/// WS Finish Callback
using ws_finish_cb = std::function<
    void(context&&)
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