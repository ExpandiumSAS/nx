#include <nx/watchers.hpp>
#include <nx/loop.hpp>

namespace nx {

after::after(timestamp timeout)
: timeout_(timeout)
{}

after&
after::operator<<(void_cb&& cb)
{
    auto ptr = new_watcher<timer>();
    auto& w = *ptr;

    w = [cb = std::move(cb), ptr]() {
        unregister_watcher(ptr);
        cb();
    };

    w.start(timeout_);

    return *this;
}

} // namespace nx
