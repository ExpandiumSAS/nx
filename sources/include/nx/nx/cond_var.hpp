#ifndef __NX_COND_VAR_H__
#define __NX_COND_VAR_H__

#include <mutex>
#include <condition_variable>
#include <vector>
#include <memory>
#include <exception>
#include <stdexcept>

#include <nx/config.h>

namespace nx {

class NX_API cond_var
{
public:
    explicit cond_var(std::size_t count = 1);

    cond_var(const cond_var& other) = delete;
    cond_var(cond_var&& other) = default;
    cond_var& operator=(const cond_var& other) = delete;
    cond_var& operator=(cond_var&& other) = default;

    bool ready();

    void reset();
    void notify();
    void notify_all();
    void wait();
    bool wait_for(uint64_t ms);

    void add_exception(std::exception_ptr ep);
    void handle_exceptions();

private:
    using mutex_type = std::recursive_mutex;
    using exception_ptrs = std::vector<std::exception_ptr>;

    std::size_t cur_ = 0;
    std::size_t count_ = 1;
    bool ready_ = false;
    mutex_type m_;
    std::condition_variable_any cv_;
    exception_ptrs eptrs_;
};

using cond_var_ptr = std::shared_ptr<cond_var>;
using cond_vars = std::vector<cond_var>;

template <typename ...Args>
cond_var_ptr
new_cond_var(Args&&... args)
{ return std::make_shared<cond_var>(std::forward<Args>(args)...); }

} // namespace nx

#endif // __NX_COND_VAR_H__
