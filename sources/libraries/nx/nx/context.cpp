#include <nx/context.hpp>
#include <nx/ws.hpp>

namespace nx {

void 
encode_frame_data(buffer& b, bool binary, const buffer& data)
{
    auto size = data.size();
    buffer frame(10);

    frame[0] = (binary) ? 0b10000010 : 0x10000001;
    if (size < 126) {
        frame[1] = (uint8_t)size;
        frame.resize(2);
    } else if (size >= 126 && size < 65536) {
        frame[1] = 0b1111110;
        frame[2] = (uint8_t)((size >> 8) & 0xFF);
        frame[3] = (uint8_t)(size & 0xFF);
        frame.resize(4);
    } else {
        frame[1] = 0b1111111;
        frame[2] = (uint8_t)((size >> 56) & 0xFF);
        frame[3] = (uint8_t)((size >> 48) & 0xFF);
        frame[4] = (uint8_t)((size >> 40) & 0xFF);
        frame[5] = (uint8_t)((size >> 32) & 0xFF);
        frame[6] = (uint8_t)((size >> 24) & 0xFF);
        frame[7] = (uint8_t)((size >> 16) & 0xFF);
        frame[8] = (uint8_t)((size >> 8) & 0xFF);
        frame[9] = (uint8_t)(size & 0xFF);
        frame.resize(10);
    }

    b << frame;
    b << data;
}


context& 
context::operator<< (const buffer& data)
{
    buffer f;
    encode_frame_data(f, true, data);
    w_ << std::move(f);
    return *this;
}

context& 
context::operator<< (const std::string& text)
{
    buffer f, data;
    data << text;
    encode_frame_data(f, false, data);
    w_ << std::move(f);
    return *this;
}

}   // namespace nx