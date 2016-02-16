#ifndef __NX_HANDLE_ERROR_H__
#define __NX_HANDLE_ERROR_H__

#include <iostream>

#include <boost/asio.hpp>

#include <nx/callbacks.hpp>
#include <nx/error_code.hpp>

namespace nx {

namespace tags {

struct on_error_tag : callback_tag {};

const on_error_tag on_error = {};

} // namespace tags

namespace asio = boost::asio;

template <typename Object>
bool
handle_error(Object& o, const char* what, const error_code& e)
{
    if (!e || e == asio::error::operation_aborted) {
        return false;
    }

    bool stop = false;

    if (e == asio::error::eof) {
        stop = true;
    } else if (!o.handler(tags::on_error)(o, e)) {
        // Unhandled error
        std::cerr << "unhandled error: " << e.message() << std::endl;
        stop = true;
    }

    if (stop) {
        o.stop();
    }

    return true;
}

} // namespace nx

#endif // __NX_HANDLE_ERROR_H__
