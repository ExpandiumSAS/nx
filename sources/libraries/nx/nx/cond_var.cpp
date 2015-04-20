#include <nx/cond_var.hpp>
#include <chrono>

namespace nx {

cond_var::cond_var(std::size_t count)
: count_(count)
{}

void
cond_var::notify()
{
    m_.lock();

    if (!ready_) {
        if (++cur_ == count_) {
            ready_ = true;
        }
    }

    m_.unlock();

    cv_.notify_one();
}

void
cond_var::notify_all()
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

bool
cond_var::wait_for(uint64_t ms)
{
    std::unique_lock<mutex_type> lk(m_);

    cv_.wait_for(lk, std::chrono::milliseconds(ms), [this]{ return ready_; });
    return ready_;
}

} // namespace nx

