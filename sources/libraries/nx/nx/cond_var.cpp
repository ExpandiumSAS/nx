#include <nx/cond_var.hpp>

namespace nx {

void
cond_var::notify()
{
    std::unique_lock<std::mutex> lk(m_);

    ready_ = true;
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

