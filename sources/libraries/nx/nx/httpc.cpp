#include <nx/httpc.hpp>

namespace nx {

http_request::http_request(const method& m, const endpoint& ep, int32_t timeout)
: timeout_(timeout),
  req_(m),
  ep_(ep)
{}

http_request::http_request(http_request&& other)
{ *this = std::move(other); }

http_request::~http_request()
{}

http_request&
http_request::operator=(http_request&& other)
{
    timeout_ = other.timeout_;
    req_ = std::move(other.req_);
    ep_ = std::move(other.ep_);
    reply_cb_ = std::move(other.reply_cb_);

    return *this;
}

http_request&
http_request::operator/(const std::string& path)
{
    req_ / path;
    return *this;
}

http_request&
http_request::operator=(reply_cb cb)
{
    reply_cb_ = cb;
    start();

    return *this;
}

void
http_request::start()
{
    if (timeout_ >= 0) { 
        sync_connect(ep_, std::move(req_), std::move(reply_cb_), timeout_);
    } else {
        async_connect(ep_, std::move(req_), std::move(reply_cb_));
    } 
}

http_request
httpc::operator()(const method& m, const endpoint& ep)
{ return http_request(m, ep, -1); }

http_request
httpc_sync::operator()(const method& m, const endpoint& ep, int32_t timeout)
{ return http_request(m, ep, timeout); }

} // namespace nx
