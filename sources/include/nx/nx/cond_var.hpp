#ifndef __NX_COND_VAR_H__
#define __NX_COND_VAR_H__

#include <mutex>
#include <condition_variable>
#include <functional>

#include <nx/config.h>

namespace nx {

using cond_cb = std::function<bool(void)>;

class NX_API cond_var
{
public:
    cond_var();
    cond_var(cond_cb&& cb);

    void operator<<(cond_cb&& cb);

    void notify();
    void wait();

private:
    bool ready_ = false;
    cond_cb cb_;
    std::mutex m_;
    std::condition_variable cv_;
};

} // namespace nx

#endif // __NX_COND_VAR_H__
