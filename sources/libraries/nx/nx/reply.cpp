#include <iostream>
#include <sstream>

#include <nx/reply.hpp>
#include <nx/utils.hpp>

namespace nx {

reply::reply()
: code_(OK),
content_length_(0)
{}

reply::reply(reply&& other)
{ *this = std::move(other); }

reply::~reply()
{}

reply&
reply::operator=(reply&& other)
{
    code_ = std::move(other.code_);
    content_length_ = other.content_length_;
    headers_ = std::move(other.headers_);
    data_.str(std::move(other.data_.str()));

    minor_version_ = other.minor_version_;
    raw_status_ = other.raw_status_;
    raw_msg_ = other.raw_msg_;
    raw_msg_len_ = other.raw_msg_len_;
    num_headers_ = other.num_headers_;
    prev_buf_len_ = other.prev_buf_len_;

    return *this;
}

reply::operator bool() const
{ return code_.code == 200; }

bool
reply::parse(buffer& b)
{
    bool parsed = false;
    num_headers_ = max_headers;

    if (!raw_headers_ptr_) {
        raw_headers_ptr_ = std::make_unique<phr_header[]>(max_headers);
    }

    int ret = phr_parse_response(
        b.data(), b.size(),
        &minor_version_,
        &raw_status_,
        &raw_msg_, &raw_msg_len_,
        raw_headers_ptr_.get(), &num_headers_,
        prev_buf_len_
    );

    if (ret > 0) {
        parsed = true;
        code_.code = (code_type) raw_status_;
        code_.status.assign(raw_msg_, raw_msg_len_);

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
        throw bad_response();
    } else if (ret != -2) {
        throw internal_client_error();
    }

    prev_buf_len_ = b.size();

    return parsed;
}

std::size_t
reply::content_length() const
{ return content_length_; }

const reply_code&
reply::code() const
{ return code_; }

std::string&
reply::h(const std::string& name)
{ return headers_[name]; }

const std::string&
reply::h(const std::string& name) const
{ return headers_[name]; }

std::ostringstream&
reply::data()
{ return data_; }

std::string
reply::content() const
{
    std::ostringstream oss;

    auto body = data_.str();
    auto hdrs = headers_;

    hdrs << header(nx::content_length, body.size());

    oss
        << "HTTP/1.1 "
        << code_.code << " " << code_.status << "\r\n"
        << hdrs
        << "\r\n"
        << body
        ;

    return oss.str();
}

bool
reply::operator==(const reply_code& rc) const
{ return code_ == rc; }

bool
reply::operator!=(const reply_code& rc) const
{ return !(*this == rc); }

bool
reply::operator==(const error& e) const
{ return code_.code == e.code(); }

bool
reply::operator!=(const error& e) const
{ return !(*this == e); }

reply&
reply::operator<<(const reply_code& rc)
{
    code_ = rc;

    return *this;
}

reply&
reply::operator<<(const error& e)
{
    code_ = reply_code{ e.code(), e.what() };

    return *this;
}

reply&
reply::operator<<(const header& h)
{
    headers_ << h;

    return *this;
}

} // namespace nx
