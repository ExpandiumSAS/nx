#ifndef __NX_REPLY_H__
#define __NX_REPLY_H__

#include <ostream>
#include <sstream>
#include <string>
#include <memory>

#include <nx/picohttpparser.h>

#include <nx/config.h>
#include <nx/context.hpp>
#include <nx/http_msg.hpp>
#include <nx/http_status.hpp>
#include <nx/handlers.hpp>

namespace nx {

class NX_API reply : public http_msg<reply>
{
public:
    reply();
    reply(const reply& other) = delete;
    reply(reply&& other);
    virtual ~reply();

    reply& operator=(reply&& other);

    operator bool() const;

    bool parse(buffer& b);
    std::string header_data() const;

    const http_status& code() const;
    bool is_error() const;
    const ws_connection& websocket_callback() const;

    void postpone();
    bool postponed();

    void done();

    bool operator==(const http_status& s) const;
    bool operator!=(const http_status& s) const;

    reply& operator|(void_cb cb);

    reply& operator<<(const http_status& s);
    reply& operator<<(const std::exception& e);

    reply& operator<<(const ws_connection& w);

    using http_msg::operator<<;

private:
    void handle_error();

    http_status status_;
    bool postponed_;
    bool upgraded_;
    void_cbs done_cbs_;
    ws_connection ws_connection_;

    int minor_version_;
    int raw_status_;
    const char* raw_msg_;
    std::size_t raw_msg_len_;
    std::size_t prev_buf_len_ = 0;
};

} // namespace nx

#endif // __NX_REPLY_H__
