#ifndef __NX_TCP_H__
#define __NX_TCP_H__

#include <nx/handle.hpp>

namespace nx {

namespace tags {

struct on_connect_tag : callback_tag {};

const on_connect_tag on_connect = {};

} // namespace tags

template <typename Derived>
class tcp_base
: public handle<
    Derived,
    callback<tags::on_connect_tag, Derived&>
>
{
};

class tcp : public tcp_base<tcp>
{
    using base_type = tcp_base<tcp>;

    using base_type::tcp_base;
};

} // namesapce nx

#endif // __NX_TCP_H__
