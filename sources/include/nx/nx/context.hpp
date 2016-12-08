#pragma once

#include <functional>

#include <nx/config.h>
#include <nx/buffer.hpp>
#include <string>

namespace nx {

NX_API void encode_frame_data(buffer& b, bool binary, const buffer& data);

class ws;

struct context {
    context(ws& w)
    : w_(w)
    {}

    context& operator<< (const buffer& data);
    context& operator<< (const std::string& text);

    ws& w_;
};

}   // namespace nx