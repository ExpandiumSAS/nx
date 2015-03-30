#include <nx/http.hpp>
#include <nx/regexp/http.hpp>

namespace nx {

http::http()
: base_type(),
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
    } catch (const error& e) {
        rep_ = e;
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
    } catch (const error& e) {
        rep_ = e;
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

const endpoint&
http::operator()(
    const endpoint& ep,
    const std::string& root,
    request_cb cb
)
{
    return
        base_type::serve(
            ep,
            [cb](nx::http& c) {
                c.request_cb_ = cb;
            },
            [](nx::http& t) {
                t.process_request();
            }
        );
}

void
http::operator()(
    const endpoint& ep,
    request&& req,
    reply_cb cb
)
{
    req_ = std::move(req);
    reply_cb_ = cb;

    handler(tags::on_read) = [](http& t) {
        t.process_reply();
    };

    base_type::connect(
        ep,
        [](http& t) {
            t.send_request();
        }
    );
}

} // namespace nx

