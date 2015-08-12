#ifndef __NX_HANDLE_ERROR_H__
#define __NX_HANDLE_ERROR_H__

#include <boost/asio.hpp>

#include <nx/callbacks.hpp>

namespace nx {

namespace tags {

struct on_error_tag : callback_tag {};

const on_error_tag on_error = {};

} // namespace tags

namespace asio = boost::asio;

using error_code = boost::system::error_code;

template <typename Object>
bool
handle_error(Object& o, const char* what, const error_code& e)
{
    if (!e || e == asio::error::operation_aborted) {
        return false;
    }

    if (!o.handler(tags::on_error)(o, e)) {
        // Unhandled error
        std::cerr << "unhandled error: " << e.message() << std::endl;
        o.stop();
    }

    return true;
}

} // namespace nx

#endif // __NX_HANDLE_ERROR_H__
