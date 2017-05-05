#pragma once

#include <functional>

#include <nx/config.h>
#include <nx/buffer.hpp>
#include <nx/json.hpp>
#include <string>
#include <memory>

namespace nx {

/// Forward declaration
class ws; 
using ws_ptr = std::shared_ptr<ws>;
using ws_weak_ptr = std::weak_ptr<ws>;

template <typename T>
using t_ptr = std::shared_ptr<T>;
template <typename T>
using t_weak_ptr = std::weak_ptr<T>;

const uint8_t text_frame_type = 0;
const uint8_t binary_frame_type = 1;

struct frame_type {
    const uint8_t value;
};

const frame_type ws_text = { text_frame_type };
const frame_type ws_binary = { binary_frame_type };
const frame_type ws_json = { text_frame_type };

NX_API void encode_frame_data(buffer& b, bool binary, const buffer& data);

/// WS contextual class
template <typename WS>
class NX_API context {
public:
    context(t_ptr<WS> w)
    : w_{w}
    {}

    ~context()
    { flush(); }

    context(const context& ) = delete;
    context(context&& ) = default;

    context& operator=(const context& ) = delete;
    context& operator=(context&& ) = default;

    context& operator<< (const frame_type& )
    { flush(); }    

    context& operator<< (const buffer& data)
    {   
        data_ << data;
        return *this;
    }

    context& operator<< (const json& j)
    {
        type_ = text_frame_type;

        (*this) << j;
        return *this;
    }

    context& operator<< (const jsonv::value& v)
    {
        type_ = text_frame_type;

        std::ostringstream oss;
        oss << v;
        (*this) << oss.str();

        return *this;
    }

    template<typename T>
    context& operator<< (T&& v)
    {
        data_ << std::forward<T>(v);
        return *this;
    }

    context& operator<< (jsonv::value&& v)
    {
        type_ = text_frame_type;

        std::ostringstream oss;
        oss << v;
        (*this) << oss.str();

        return *this;
    }

    void stop()
    { 
/*        async() << [this]() {
            if (auto w = w_.lock()) {
                w->stop();
            }
        }; */
    }

    std::string uid()
    {
        std::string result;
        if (auto w = w_.lock()) {
            result = w->uid();
        }
        return result;
    }

    std::string uid() const
    {
        std::string result;
        if (auto w = w_.lock()) {
            result = w->uid();
        }
        return result;
    }

    bool operator< (const nx::context<WS>& rhs) const
    { return uid() > rhs.uid(); }

    void flush()
    {
        if (!data_.empty()) {
            buffer f;
            encode_frame_data(f, type_ == binary_frame_type, data_);
            if (auto w = w_.lock()) {
                (*w) << std::move(f);
            }
            data_.clear();
        }
    }

private:
    t_weak_ptr<WS> w_;
    buffer data_;
    uint8_t type_{ text_frame_type };
};


/// WS Connection Callback 
using ws_connect_cb = std::function<
    void(context<ws>&&)
>;

/// WS Message Callback
using ws_message_cb = std::function<
    void(context<ws>&&, const buffer& data)
>;

/// WS Finish Callback
using ws_finish_cb = std::function<
    void(context<ws>&&)
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

}   // namespace nx