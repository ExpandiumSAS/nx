#ifndef __NX_LOOP_H__
#define __NX_LOOP_H__

#include <thread>
#include <mutex>
#include <condition_variable>

#include <nx/config.h>

namespace nx {

class NX_API loop
{
public:
    static loop& get();

    void start();
    void stop();

private:
    loop();
    ~loop();

    loop(const loop&) = delete;
    loop(loop&&) = delete;
    void operator=(const loop&) = delete;
    void operator=(loop&&) = delete;

    void lock();
    void unlock();

    std::mutex m_;
    std::condition_variable cv_;
    std::thread t_;
};

} // namespace nx

#endif // __NX_LOOP_H__
