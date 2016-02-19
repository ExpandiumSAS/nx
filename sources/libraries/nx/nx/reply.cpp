#include <iostream>
#include <sstream>

#include <nx/reply.hpp>
#include <nx/utils.hpp>

namespace nx {

reply::reply()
: status_(OK),
postponed_(false)
{}

reply::reply(reply&& other)
{ *this = std::move(other); }

reply::~reply()
{}

reply&
reply::operator=(reply&& other)
{
    http_msg::operator=(std::forward<reply>(other));
    status_ = std::move(other.status_);

    postponed_ = other.postponed_;
    done_cb_ = std::move(other.done_cb_);

    minor_version_ = other.minor_version_;
    raw_status_ = other.raw_status_;
    raw_msg_ = other.raw_msg_;
    raw_msg_len_ = other.raw_msg_len_;
    prev_buf_len_ = other.prev_buf_len_;

    return *this;
}

reply::operator bool() const
{ return status_.code == 200; }

bool
reply::parse(buffer& b)
{
    bool parsed = false;

    pre_parse();

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
        post_parse();
        b.erase(b.begin(), b.begin() + (std::size_t) ret);
    } else if (ret == -1) {
        throw BadResponse;
    } else if (ret != -2) {
        throw InternalClientError;
    }

    prev_buf_len_ = b.size();

    return parsed;
}

const http_status&
reply::code() const
{ return status_; }

std::string
reply::header_data() const
{
    std::ostringstream oss;

    auto hdrs = headers_;

    hdrs << header(nx::Content_Length, std::to_string(data_.size()));

    oss
        << "HTTP/1.1 "
        << status_.code << " " << status_.status << "\r\n"
        << hdrs
        << "\r\n"
        ;

    return oss.str();
}

void
reply::postpone()
{ postponed_ = true; }

bool
reply::postponed()
{ return postponed_; }

void
reply::done()
{
    if (done_cb_) {
        done_cb_();
    }
}

void_cb&
reply::on_done()
{ return done_cb_; }

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

    if (s.is_error()) {
        data_.clear();
        jsonv::value e = jsonv::object({{ "error", s.error }});
        *this << e;
    }

    return *this;
}

} // namespace nx
