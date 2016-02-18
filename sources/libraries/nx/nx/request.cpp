#include <iostream>
#include <cctype>
#include <algorithm>

#include <nx/request.hpp>
#include <nx/http_status.hpp>
#include <nx/utils.hpp>

namespace nx {

request::request()
: http_msg()
{}

request::request(const nx::method& m)
: http_msg(),
method_(m.str)
{}

request::request(const std::string& method)
: http_msg(),
method_(method)
{}

request::request(const std::string& method, const std::string& path)
: http_msg(),
method_(method),
path_(path)
{}

request::request(request&& other)
{ *this = std::move(other); }

request::~request()
{}

request&
request::operator=(request&& other)
{
    http_msg::operator=(std::forward<request>(other));
    method_ = std::move(other.method_);
    path_ = std::move(other.path_);
    attrs_ = std::move(other.attrs_);

    raw_method_= other.raw_method_;
    raw_method_len_= other.raw_method_len_;
    raw_path_= other.raw_path_;
    raw_path_len_= other.raw_path_len_;

    return *this;
}

bool
request::parse(buffer& b)
{
    bool parsed = false;

    pre_parse();

    int ret = phr_parse_request(
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

        post_parse();

        b.erase(b.begin(), b.begin() + (std::size_t) ret);
    } else if (ret == -1) {
        throw BadRequest;
    } else if (ret != -2) {
        throw InternalServerError;
    }

    prev_buf_len_ = b.size();

    return parsed;
}

std::string
request::header_data() const
{
    std::ostringstream oss;

    auto hdrs = headers_;

    if (method_ != GET.str) {
        hdrs << header(nx::Content_Length, std::to_string(data_.size()));
    }

    oss
        << method_ << " " << clean_path(path_) << " HTTP/1.1\r\n"
        << hdrs
        << "\r\n"
        ;

    return oss.str();
}

const std::string&
request::method() const
{ return method_; }

const std::string&
request::path() const
{ return path_; }

std::string&
request::a(const std::string& name)
{ return attrs_[name]; }

const std::string&
request::a(const std::string& name) const
{ return attrs_[name]; }

bool
request::operator==(const nx::method& m) const
{ return method_ == m.str; }

bool
request::operator!=(const nx::method& m) const
{ return !(*this == m); }

request&
request::operator/(const std::string& path)
{
    path_ += '/';
    path_ += path;

    return *this;
}

request&
request::operator<<(const nx::method& m)
{
    method_ = m.str;

    return *this;
}

request&
request::operator<<(const attribute& a)
{
    attrs_ << a;

    return *this;
}

request&
request::operator<<(const attributes& a)
{
    attrs_ << a;

    return *this;
}

bool
request::is_form() const
{
    return
        *this == POST
        &&
        h(content_type) == "application/x-www-form-urlencoded"
        ;
}

} // namespace nx
