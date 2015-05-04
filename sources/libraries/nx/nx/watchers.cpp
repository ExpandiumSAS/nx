#include <nx/watchers.hpp>

namespace nx {

after::after(timestamp timeout)
: timeout_(timeout)
{}

after&
after::operator<<(void_cb&& cb)
{
    async() << [cb = std::move(cb), timeout_](evloop el) {
        ev_once(
        );
    };

    return *this;
}

} // namespace nx
