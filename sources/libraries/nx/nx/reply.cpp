#include <iostream>
#include <sstream>

#include <nx/reply.hpp>
#include <nx/utils.hpp>
#include <cxxu/logging.hpp>

namespace nx {

reply::reply()
: status_(OK),
postponed_(false),
upgraded_(false)
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
    upgraded_ = other.upgraded_;
    done_cbs_ = std::move(other.done_cbs_);

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

bool
reply::is_error() const
{ return status_.is_error(); }

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
reply::upgrade()
{ upgraded_ = true; }

bool
reply::upgraded()
{ return upgraded_; }

void
reply::done()
{
    while (!done_cbs_.empty()) {
        auto cb = std::move(done_cbs_.back());
        done_cbs_.pop_back();

        try {
            cb();
        } catch (const std::exception& e) {
            cxxu::error()
                << "reply callback failed (ignored): "
                << e.what()
                ;
        }
    }
}

bool
reply::operator==(const http_status& s) const
{ return status_ == s; }

bool
reply::operator!=(const http_status& s) const
{ return !(*this == s); }

reply&
reply::operator|(void_cb cb)
{
    done_cbs_.emplace_back(cb);

    return *this;
}

reply&
reply::operator<<(const http_status& s)
{
    status_ = s;
    handle_error();

    return *this;
}

reply&
reply::operator<<(const std::exception& e)
{ return (*this) << BadRequest(e); }

void
reply::handle_error()
{
    if (status_.is_error()) {
        data_.clear();
        jsonv::value e = jsonv::object({{ "error", status_.error }});
        *this << e;
    }
}

} // namespace nx
