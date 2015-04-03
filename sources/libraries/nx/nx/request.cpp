#include <iostream>
#include <cctype>
#include <algorithm>

#include <nx/request.hpp>
#include <nx/http_status.hpp>
#include <nx/utils.hpp>

namespace nx {

request::request()
: content_length_(0)
{}

request::request(const nx::method& m)
: method_(m.str),
content_length_(0)
{}

request::request(const std::string& method)
: method_(method),
content_length_(0)
{}

request::request(const std::string& method, const std::string& path)
: method_(method),
path_(path),
content_length_(0)
{}

request::request(request&& other)
{ *this = std::move(other); }

request::~request()
{}

request&
request::operator=(request&& other)
{
    raw_headers_ptr_ = std::move(other.raw_headers_ptr_);
    headers_ = std::move(other.headers_);
    method_ = std::move(other.method_);
    path_ = std::move(other.path_);
    content_length_ = other.content_length_;
    attrs_ = std::move(other.attrs_);
    data_.str(std::move(other.data_.str()));

    raw_method_= other.raw_method_;
    raw_method_len_= other.raw_method_len_;
    raw_path_= other.raw_path_;
    raw_path_len_= other.raw_path_len_;
    minor_version_= other.minor_version_;
    num_headers_= other.num_headers_;
    prev_buf_len_= other.prev_buf_len_;

    return *this;
}

bool
request::parse(buffer& b)
{
    bool parsed = false;
    num_headers_ = max_headers;

    if (!raw_headers_ptr_) {
        raw_headers_ptr_ = std::make_unique<phr_header[]>(max_headers);
    }

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

        for (std::size_t i = 0; i < num_headers_; i++) {
            auto& h = raw_headers_ptr_.get()[i];

            std::string name(h.name, h.name_len);
            std::string value(h.value, h.value_len);

            headers_ << header(std::move(name), std::move(value));
        }

        // Grab common useful header values
        if (headers_.has(content_length_lc)) {
            content_length_ = to_num<std::size_t>(headers_[content_length_lc]);
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
request::method() const
{ return method_; }

const std::string&
request::path() const
{ return path_; }

std::size_t
request::content_length() const
{ return content_length_; }

std::string&
request::h(const std::string& name)
{ return headers_[name]; }

const std::string&
request::h(const std::string& name) const
{ return headers_[name]; }

std::string&
request::a(const std::string& name)
{ return attrs_[name]; }

const std::string&
request::a(const std::string& name) const
{ return attrs_[name]; }

std::ostringstream&
request::data()
{ return data_; }

std::string
request::content() const
{
    std::ostringstream oss;

    auto body = data_.str();
    auto hdrs = headers_;

    if (method_ != GET.str) {
        hdrs << header(nx::content_length, std::to_string(body.size()));
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
request::operator<<(const header& h)
{
    headers_ << h;

    return *this;
}

request&
request::operator<<(const headers& h)
{
    headers_ << h;

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

request&
request::operator<<(const json& js)
{
    headers_ << application_json;
    data_ << js;

    return *this;
}

request&
request::operator<<(const jsonv::value& jv)
{
    headers_ << application_json;
    data_ << jv.as_string();

    return *this;
}

bool
request::is(const nx::method& m) const
{ return method_ == m.str; }

bool
request::is_form() const
{
    return
        is(POST)
        &&
        h(content_type_lc) == "application/x-www-form-urlencoded"
        ;
}

} // namespace nx
