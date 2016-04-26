#include <chrono>
#include <sstream>

#include <nx/cond_var.hpp>

namespace nx {

cond_var::cond_var(std::size_t count)
: count_(count)
{}

bool
cond_var::ready()
{
    bool val = false;

    m_.lock();
    val = ready_;
    m_.unlock();

    return val;
}

void
cond_var::reset()
{
    m_.lock();
    cur_ = 0;
    ready_ = false;
    eptrs_.clear();
    m_.unlock();
}

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

    cv_.notify_all();
}

void
cond_var::notify_all()
{
    m_.lock();
    ready_ = true;
    m_.unlock();

    cv_.notify_all();
}

void
cond_var::wait()
{
    std::unique_lock<mutex_type> lk(m_);

    cv_.wait(lk, [this]{ return ready_; });
    handle_exceptions();
}

bool
cond_var::wait_for(uint64_t ms)
{
    std::unique_lock<mutex_type> lk(m_);

    cv_.wait_for(lk, std::chrono::milliseconds(ms), [this]{ return ready_; });

    if (ready_) {
        handle_exceptions();
    }

    return ready_;
}

void
cond_var::add_exception(std::exception_ptr ep)
{
    std::unique_lock<mutex_type> lk(m_);

    eptrs_.push_back(ep);
}

void
cond_var::handle_exceptions()
{
    if (eptrs_.empty()) {
        return;
    }

    std::ostringstream oss;

    oss
        << eptrs_.size() << " exception"
        << (eptrs_.size() > 1 ? "s" : "")
        << " occured while waiting:\n"
        ;

    std::size_t pos = 0;

    for (auto& eptr : eptrs_) {
        oss << (pos > 0 ? "\n" : "") << "[" << pos << "]: ";

        if (eptr) {
            try {
                std::rethrow_exception(eptr);
            } catch (const std::exception& e) {
                oss << e.what();
            }
        } else {
            oss << "(empty exception)";
        }

        pos++;
    }

    throw std::runtime_error(oss.str());
}

} // namespace nx

