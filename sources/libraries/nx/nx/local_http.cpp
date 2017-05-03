#include <nx/local_http.hpp>
#include <nx/local_ws.hpp>

namespace nx {

local_http::local_http(request&& req, reply_cb&& cb)
: req_(std::move(req)),
reply_cb_(std::move(cb))
{}

local_http::local_http(request&& req, reply_cb&& cb, asio::io_service& io)
: base_type(io),
  req_(std::move(req)),
  reply_cb_(std::move(cb))
{}

local_http::local_http(reply&& rep, request_cb&& cb)
: rep_(std::move(rep)),
request_cb_(std::move(cb))
{}

bool
local_http::request_parsed()
{
    if (!parsed_) {
        parsed_ = req_.parse(rbuf());
    }

    return parsed_;
}

bool
local_http::reply_parsed()
{
    if (!parsed_) {
        parsed_ = rep_.parse(rbuf());
    }

    return parsed_;
}

void
local_http::call_or_fail(void_cb cb)
{
    try {
        cb();
    } catch (const http_status& s) {
        rep_ << s;
    } catch (const std::exception& e) {
        std::cout << "BadRequest by " << e.what() << std::endl;
        rep_ << BadRequest(e);
    }
}

void
local_http::process_request()
{
    call_or_fail(
        [&]() {
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

            auto self = ptr();
            if (req_.is_upgrade()) {
                local_ws::server_handshake(req_, rep_);

                rep_ | [this,self]() mutable {
                    *this << rep_;

                    process_upgrade();
                };

                 // All data arrived, call upper handler
                request_cb_(req_, rbuf(), rep_);
            } else {
                // Register callback to be called when reply is ready to send
                // (will be called last by rep.done())
                rep_ << connection_close;

                rep_ | [this,self]() mutable {
                    *this << rep_;
                    close_after_write();
                    self.reset();
                };

                // All data arrived, call upper handler
                request_cb_(req_, rbuf(), rep_);
            }
        }
    );

    if (!rep_.postponed()) {
        rep_.done();
    }
}

void
local_http::process_upgrade()
{
    auto self = ptr();
    async() << [this,self]() {
        this->cancel();

        auto& w = this->upgrade_connection<local_ws>();
        w.set_callbacks(rep_.websocket_callback());

        this->dispose();
        w.start();
    };
}

bool
local_http::process_reply()
{
    try {
        if (!reply_parsed() || rbuf().size() < rep_.content_length()) {
            // Wait until response is complete
            return false;
        }
    } catch (const http_status& s) {
        rep_ << s;
    } catch (const std::exception& e) {
        rep_ << BadResponse(e);
    }

    // All data arrived, call upper handler
    reply_cb_(rep_, rbuf());
    close();
    return true;
}

void
local_http::send_request()
{
    req_ << header("Host", local_str());
    *this << std::move(req_);
}

local_http&
local_http::operator<<(request_cb cb)
{
    request_cb_ = std::move(cb);

    return *this;
}

local_http&
local_http::operator<<(reply_cb cb)
{
    reply_cb_ = std::move(cb);

    return *this;
}

} // namespace nx
