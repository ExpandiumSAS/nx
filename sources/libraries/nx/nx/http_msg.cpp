#include <iostream>
#include <cctype>
#include <algorithm>

#include <nx/http_msg.hpp>
#include <nx/utils.hpp>

namespace nx {

http_msg_base::http_msg_base()
: content_length_(0)
{}

http_msg_base::http_msg_base(http_msg_base&& other)
{ *this = std::move(other); }

http_msg_base::~http_msg_base()
{}

http_msg_base&
http_msg_base::operator=(http_msg_base&& other)
{
    raw_headers_ptr_ = std::move(other.raw_headers_ptr_);
    headers_ = std::move(other.headers_);
    content_length_ = other.content_length_;
    data_ = std::move(other.data_);

    minor_version_ = other.minor_version_;
    num_headers_= other.num_headers_;
    prev_buf_len_= other.prev_buf_len_;

    return *this;
}

void
http_msg_base::pre_parse()
{
    num_headers_ = max_headers;

    if (!raw_headers_ptr_) {
        raw_headers_ptr_ = std::make_unique<phr_header[]>(max_headers);
    }
}

void
http_msg_base::post_parse()
{
    for (std::size_t i = 0; i < num_headers_; i++) {
        auto& h = raw_headers_ptr_.get()[i];

        std::string name(h.name, h.name_len);
        std::string value(h.value, h.value_len);

        headers_ << header(std::move(name), std::move(value));
    }

    // Grab common useful header values
    if (headers_.has(nx::content_length)) {
        content_length_ = to_num<std::size_t>(headers_[nx::content_length]);
    }

    raw_headers_ptr_.reset();
}

std::size_t
http_msg_base::content_length() const
{ return content_length_; }

const nx::data&
http_msg_base::data() const
{ return data_; }

std::string&
http_msg_base::h(const std::string& name)
{ return headers_[name]; }

const std::string&
http_msg_base::h(const std::string& name) const
{ return headers_[name]; }

bool
http_msg_base::has(const header& h) const
{ return headers_.has(h.name) && headers_[h.name] == h.value; }

bool
http_msg_base::has(const std::string& name) const
{ return headers_.has(name); }

http_msg_base&
http_msg_base::operator<<(const header& h)
{
    headers_ << h;

    return *this;
}

http_msg_base&
http_msg_base::operator<<(const headers& h)
{
    headers_ << h;

    return *this;
}

http_msg_base&
http_msg_base::operator<<(const json& js)
{
    headers_ << application_json;
    data_ << js;

    return *this;
}

http_msg_base&
http_msg_base::operator<<(const jsonv::value& v)
{
    headers_ << application_json;

    std::ostringstream oss;
    oss << v;
    data_ << oss.str();

    return *this;
}

} // namespace nx
