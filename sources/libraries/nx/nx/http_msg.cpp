#include <iostream>
#include <cctype>
#include <algorithm>

#include <nx/http_msg.hpp>
#include <nx/utils.hpp>

namespace nx {

http_msg::http_msg()
: content_length_(0)
{}

http_msg::http_msg(http_msg&& other)
{ *this = std::move(other); }

http_msg::~http_msg()
{}

http_msg&
http_msg::operator=(http_msg&& other)
{
    raw_headers_ptr_ = std::move(other.raw_headers_ptr_);
    headers_ = std::move(other.headers_);
    content_length_ = other.content_length_;
    data_.str(std::move(other.data_.str()));

    minor_version_ = other.minor_version_;
    num_headers_= other.num_headers_;
    prev_buf_len_= other.prev_buf_len_;

    return *this;
}

bool
http_msg::parse(buffer& b)
{
    bool parsed = false;
    num_headers_ = max_headers;

    if (!raw_headers_ptr_) {
        raw_headers_ptr_ = std::make_unique<phr_header[]>(max_headers);
    }

    int ret = phr_parse_http_msg(
        b.data(), b.size(),
        &raw_method_, &raw_method_len_,
        &raw_path_, &raw_path_len_,
        &minor_version_,
        raw_headers_ptr_.get(), &num_headers_,
        prev_buf_len_
    );

    if (ret > 0) {
        parsed = true;
        method_.assign(raw_method_, raw_method_len_);

        uri u(std::string(raw_path_, raw_path_len_));
        path_ = std::move(u.path());
        attrs_ = std::move(u.a());

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
        b.erase(b.begin(), b.begin() + (std::size_t) ret);
    } else if (ret == -1) {
        throw BadRequest;
    } else if (ret != -2) {
        throw InternalServerError;
    }

    prev_buf_len_ = b.size();

    return parsed;
}

const std::string&
http_msg::method() const
{ return method_; }

const std::string&
http_msg::path() const
{ return path_; }

std::size_t
http_msg::content_length() const
{ return content_length_; }

std::string&
http_msg::h(const std::string& name)
{ return headers_[name]; }

const std::string&
http_msg::h(const std::string& name) const
{ return headers_[name]; }

bool
http_msg::has(const header& h) const
{ return headers_.has(h.name) && headers_[h.name] == h.value; }

bool
http_msg::has(const std::string& name) const
{ return headers_.has(name); }

std::string&
http_msg::a(const std::string& name)
{ return attrs_[name]; }

const std::string&
http_msg::a(const std::string& name) const
{ return attrs_[name]; }

std::string
http_msg::content() const
{
    std::ostringstream oss;

    auto body = data_.str();
    auto hdrs = headers_;

    if (method_ != GET.str) {
        hdrs << header(nx::Content_Length, std::to_string(body.size()));
    }

    oss
        << method_ << " " << clean_path(path_) << " HTTP/1.1\r\n"
        << hdrs
        << "\r\n"
        ;

    if (method_ != GET.str) {
        oss << body;
    }

    return oss.str();
}

bool
http_msg::operator==(const nx::method& m) const
{ return method_ == m.str; }

bool
http_msg::operator!=(const nx::method& m) const
{ return !(*this == m); }

http_msg&
http_msg::operator/(const std::string& path)
{
    path_ += '/';
    path_ += path;

    return *this;
}

http_msg&
http_msg::operator<<(const nx::method& m)
{
    method_ = m.str;

    return *this;
}

http_msg&
http_msg::operator<<(const header& h)
{
    headers_ << h;

    return *this;
}

http_msg&
http_msg::operator<<(const headers& h)
{
    headers_ << h;

    return *this;
}

http_msg&
http_msg::operator<<(const attribute& a)
{
    attrs_ << a;

    return *this;
}

http_msg&
http_msg::operator<<(const attributes& a)
{
    attrs_ << a;

    return *this;
}

http_msg&
http_msg::operator<<(const json& js)
{
    headers_ << application_json;
    data_ << js;

    return *this;
}

http_msg&
http_msg::operator<<(const jsonv::value& v)
{
    headers_ << application_json;

    std::ostringstream oss;
    oss << v;
    data_ << oss.str();

    return *this;
}

bool
http_msg::is_form() const
{
    return
        *this == POST
        &&
        h(content_type) == "application/x-www-form-urlencoded"
        ;
}

} // namespace nx
