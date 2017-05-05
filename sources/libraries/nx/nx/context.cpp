#include <nx/context.hpp>
#include <nx/ws.hpp>

namespace nx {

void 
encode_frame_data(buffer& b, bool binary, const buffer& data)
{
    auto size = data.size();
    buffer frame(10);

    frame[0] = (binary) ? 0b10000010 : 0b10000001;
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

context::~context()
{ flush(); }

context& 
context::operator<< (const frame_type& type)
{
    type_ = type.value;
    return *this;
}

context& 
context::operator<< (const buffer& data)
{   
    data_ << data;
    return *this;
}

context& 
context::operator<< (const json& j)
{
    type_ = text_frame_type;

    (*this) << j;
    return *this;
}


context& 
context::operator<< (const jsonv::value& v)
{
    type_ = text_frame_type;

    std::ostringstream oss;
    oss << v;
    (*this) << oss.str();

    return *this;
}

context& 
context::operator<< (jsonv::value&& v)
{
    type_ = text_frame_type;

    std::ostringstream oss;
    oss << v;
    (*this) << oss.str();

    return *this;
}

void
context::stop()
{ 
    async() << [this]() {
        if (auto w = w_.lock()) {
            w->stop_socket();
        }
    }; 
}

std::string 
context::uid()
{
    std::string result;
    if (auto w = w_.lock()) {
        result = w->uid();
    }
    return result;
}

std::string
context::uid() const
{
    std::string result;
    if (auto w = w_.lock()) {
        result = w->uid();
    }
    return result;
}

bool 
context::operator< (const context& rhs) const
{ return uid() > rhs.uid(); }

void
context::flush()
{
    if (!data_.empty()) {
        buffer f;
        encode_frame_data(f, type_ == binary_frame_type, data_);
        if (auto w = w_.lock()) {
            //(*w) << std::move(f);
            w->push_in_socket(std::move(f));
        }
        data_.clear();
    }
}

}   // namespace nx