#include <nx/cond_var.hpp>

namespace nx {

cond_var::cond_var()
: cb_(nullptr)
{}

cond_var::cond_var(cond_cb&& cb)
: cb_(std::move(cb))
{}

void
cond_var::operator<<(cond_cb&& cb)
{
    std::unique_lock<std::mutex> lk(m_);
    cb_ = std::move(cb);
}

void
cond_var::notify()
{
    std::unique_lock<std::mutex> lk(m_);

    ready_ = cb_ ? cb_() : true;
    lk.unlock();
    cv_.notify_one();
}

void
cond_var::wait()
{
    std::unique_lock<std::mutex> lk(m_);

    cv_.wait(lk, [this]{ return ready_; });
}

} // namespace nx

