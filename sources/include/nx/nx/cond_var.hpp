#ifndef __NX_COND_VAR_H__
#define __NX_COND_VAR_H__

#include <mutex>
#include <condition_variable>

#include <nx/config.h>

namespace nx {

class NX_API cond_var
{
public:
    void notify();
    void wait();

private:
    bool ready_ = false;
    std::mutex m_;
    std::condition_variable cv_;
};

} // namespace nx

#endif // __NX_COND_VAR_H__
