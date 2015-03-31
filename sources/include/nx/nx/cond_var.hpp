#ifndef __NX_COND_VAR_H__
#define __NX_COND_VAR_H__

#include <mutex>
#include <condition_variable>

#include <nx/config.h>

namespace nx {

class NX_API cond_var
{
public:
    cond_var();

    void notify();
    void wait();

private:
    using mutex_type = std::recursive_mutex;

    bool ready_ = false;
    mutex_type m_;
    std::condition_variable_any cv_;
};

} // namespace nx

#endif // __NX_COND_VAR_H__
