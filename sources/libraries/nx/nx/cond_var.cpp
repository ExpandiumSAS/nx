#include <nx/cond_var.hpp>

namespace nx {

cond_var::cond_var()
{}

void
cond_var::notify()
{
    m_.lock();
    ready_ = true;
    m_.unlock();

    cv_.notify_one();
}

void
cond_var::wait()
{
    std::unique_lock<mutex_type> lk(m_);

    cv_.wait(lk, [this]{ return ready_; });
}

} // namespace nx

