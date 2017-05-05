#ifndef __NX_IWS_H__
#define __NX_IWS_H__

#include <functional>

#include <stdint.h>
#include <uuid/uuid.h>

#include <nx/config.h>
#include <nx/buffer.hpp>

namespace nx {

/// Forward declearation
class context;
struct ws_connection;

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

struct NX_API ws_frame
{
    bool fin;
    bool rsv1;
    bool rsv2;
    bool rsv3;
    uint8_t opcode;
    buffer payload;
};

class NX_API iws
{
public:
    virtual ~iws() = default;

    virtual void start() = 0;

    virtual bool parse_frame(ws_frame&) = 0;
    virtual void process_frames() = 0;
    virtual void process_close() = 0;

    virtual void set_callbacks(const ws_connection&) = 0;

    virtual std::string uid() = 0;
    virtual const std::string& uid() const = 0;


    virtual void stop_socket() = 0;
    
    virtual void push_in_socket(buffer&& b) = 0;
    virtual void push_in_socket(std::string&& s) = 0;
    virtual void push_in_socket(std::string& s) = 0;
};

} // namespace nx

#endif // __NX_WS_H__
