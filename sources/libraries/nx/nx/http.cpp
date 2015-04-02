#include <nx/http.hpp>
#include <nx/regexp/http.hpp>

namespace nx {

http::http()
: base_type(),
parsed_(false)
{}

http::http(int fh)
: base_type(fh),
parsed_(false)
{}

http::http(http&& other)
: base_type(std::forward<base_type>(other))
{ *this = std::move(other); }

http::~http()
{}

http&
http::operator=(http&& other)
{
    base_type::operator=(std::forward<base_type>(other));
    parsed_ = other.parsed_;
    req_ = std::move(other.req_);
    rep_ = std::move(other.rep_);
    request_cb_ = std::move(other.request_cb_);
    reply_cb_ = std::move(other.reply_cb_);

    return *this;
}

bool
http::request_parsed()
{
    if (!parsed_) {
        parsed_ = req_.parse(rbuf());
    }

    return parsed_;
}

bool
http::reply_parsed()
{
    if (!parsed_) {
        parsed_ = rep_.parse(rbuf());
    }

    return parsed_;
}

void
http::process_request()
{
    try {
        if (!request_parsed() || rbuf().size() < req_.content_length()) {
            // Wait until request is complete
            return;
        }

        if (req_.is_form()) {
            // Decode additional variables from body
            // TODO: implement iterator based function to avoid copy
            std::string body;
            rbuf() >> body;
            req_ << attributes(body, '&');
        }

        // All data arrived, call upper handler
        request_cb_(req_, rbuf(), rep_);
    } catch (const http_status& s) {
        rep_ << s;
    } catch (const std::exception& e) {
        std::cout << "NotFound by " << e.what() << std::endl;
        rep_ << NotFound;
    }

    *this << rep_.content();
    push_close();
}

void
http::process_reply()
{
    try {
        if (!reply_parsed() || rbuf().size() < rep_.content_length()) {
            // Wait until response is complete
            return;
        }
    } catch (const http_status& s) {
        rep_ << s;
    } catch (const std::exception& e) {
        rep_ << NotFound;
    }

    // All data arrived, call upper handler
    reply_cb_(rep_, rbuf());
    push_close();
}

void
http::send_request()
{
    req_ << header("Host", local().str());
    *this << req_.content();
}

http&
http::operator<<(request_cb cb)
{
    request_cb_ = std::move(cb);

    return *this;
}

http&
http::operator<<(request req)
{
    req_ = std::move(req);

    return *this;
}

http&
http::operator<<(reply_cb cb)
{
    reply_cb_ = std::move(cb);

    return *this;
}

http&
http::operator<<(reply rep)
{
    rep_ = std::move(rep);

    return *this;
}

} // namespace nx

