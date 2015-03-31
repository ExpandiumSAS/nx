#include <nx/httpc.hpp>

namespace nx {

httpc::httpc(const method& m, const endpoint& ep)
: req_(m),
ep_(ep)
{}

httpc::httpc(httpc&& other)
{ *this = std::move(other); }

httpc::~httpc()
{}

httpc&
httpc::operator=(httpc&& other)
{
    req_ = std::move(other.req_);
    ep_ = std::move(other.ep_);
    reply_cb_ = std::move(other.reply_cb_);

    return *this;
}

httpc&
httpc::operator/(const std::string& path)
{
    req_ / path;
    return *this;
}

httpc&
httpc::operator=(reply_cb cb)
{
    reply_cb_ = cb;
    start();

    return *this;
}

void
httpc::start()
{ connect(ep_, std::move(req_), std::move(reply_cb_)); }

} // namespace nx
