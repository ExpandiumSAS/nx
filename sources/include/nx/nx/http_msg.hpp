#ifndef __NX_HTTP_MSG_H__
#define __NX_HTTP_MSG_H__

#include <string>
#include <ostream>
#include <sstream>
#include <memory>
#include <type_traits>

#include <nx/picohttpparser.h>

#include <nx/config.h>
#include <nx/buffer.hpp>
#include <nx/headers.hpp>
#include <nx/json.hpp>
#include <nx/file.hpp>
#include <nx/data.hpp>
#include <nx/socket_base.hpp>

namespace nx {

class NX_API http_msg
{
public:
    http_msg();
    http_msg(const http_msg& other) = delete;
    http_msg(http_msg&& other);
    virtual ~http_msg();

    http_msg& operator=(http_msg&& other);

    bool parse(buffer& b);

    std::size_t content_length() const;

    std::string& h(const std::string& name);
    const std::string& h(const std::string& name) const;
    bool has(const header& h) const;
    bool has(const std::string& name) const;

    http_msg& operator<<(const header& h);
    http_msg& operator<<(const headers& h);
    http_msg& operator<<(const json& js);
    http_msg& operator<<(const jsonv::value& v);
    http_msg& operator<<(const file& f);

    template <typename T>
    http_msg& operator<<(const T& v)
    {
        data_ << v;

        return *this;
    }

    bool is_form() const;

protected:
    using raw_headers_ptr = std::unique_ptr<phr_header[]>;
    static const std::size_t max_headers = 128;

    raw_headers_ptr raw_headers_ptr_;
    headers headers_;
    std::size_t content_length_;
    nx::data data_;
    std::string empty_;

    int minor_version_;
    std::size_t num_headers_;
    std::size_t prev_buf_len_ = 0;
};

} // namespace nx

#endif // __NX_HTTP_MSG_H__
