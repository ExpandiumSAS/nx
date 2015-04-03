#include <iostream>
#include <sstream>

#include <nx/reply.hpp>
#include <nx/utils.hpp>

namespace nx {

reply::reply()
: status_(OK),
content_length_(0)
{}

reply::reply(reply&& other)
{ *this = std::move(other); }

reply::~reply()
{}

reply&
reply::operator=(reply&& other)
{
    status_ = std::move(other.status_);
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
{ return status_.code == 200; }

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
        status_.code = raw_status_;
        status_.status.assign(raw_msg_, raw_msg_len_);

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
        throw BadResponse;
    } else if (ret != -2) {
        throw InternalClientError;
    }

    prev_buf_len_ = b.size();

    return parsed;
}

std::size_t
reply::content_length() const
{ return content_length_; }

const http_status&
reply::code() const
{ return status_; }

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

    hdrs << header(nx::content_length, std::to_string(body.size()));

    oss
        << "HTTP/1.1 "
        << status_.code << " " << status_.status << "\r\n"
        << hdrs
        << "\r\n"
        << body
        ;

    return oss.str();
}

bool
reply::operator==(const http_status& s) const
{ return status_ == s; }

bool
reply::operator!=(const http_status& s) const
{ return !(*this == s); }

reply&
reply::operator<<(const http_status& s)
{
    status_ = s;

    return *this;
}

reply&
reply::operator<<(const header& h)
{
    headers_ << h;

    return *this;
}

reply&
reply::operator<<(const jsonv::value& v)
{
    std::ostringstream os;

    os << v;
    data_ << os.str();

    return *this;
}

} // namespace nx
