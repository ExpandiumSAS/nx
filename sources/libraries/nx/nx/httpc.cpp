#include <hxx/httpc.hpp>

namespace hxx {

client::client(const method& m, const endpoint& ep, done_cb cb)
: req_(m),
ep_(ep),
done_cb_(cb)
{}

client::client(client&& other)
{ *this = std::move(other); }

client::~client()
{}

client&
client::operator=(client&& other)
{
    http_ = std::move(other.http_);
    req_ = std::move(other.req_);
    ep_ = std::move(other.ep_);
    reply_cb_ = std::move(other.reply_cb_);
    done_cb_ = std::move(other.done_cb_);

    return *this;
}

client&
client::operator/(const std::string& path)
{
    req_ / path;
    return *this;
}

client&
client::operator=(reply_cb cb)
{
    reply_cb_ = cb;
    start();

    return *this;
}

void
client::start()
{
    http_.handler(tags::on_closed) =
        [cb = std::move(done_cb_)](http& t) {
            async() << [cb = std::move(cb)]() {
                cb();
            };
        };

    http_(ep_, std::move(req_), reply_cb_);
}

httpc::httpc()
: next_id_(0)
{}

httpc::httpc(httpc&& other)
{ *this = std::move(other); }

httpc::~httpc()
{}

httpc&
httpc::operator=(httpc&& other)
{
    next_id_ = other.next_id_;
    clients_ = std::move(other.clients_);

    return *this;
}

client&
httpc::operator()(const method& m, const endpoint& ep)
{
    auto id = ++next_id_;

    auto p = clients_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(id),
        std::forward_as_tuple(
            m, ep,
            [this,id]() {
                clients_.erase(id);
            }
        )
    );

    client& c = p.first->second;

    return c;
}

} // namespace hxx
