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

class NX_API http_msg_base
{
public:
    http_msg_base();
    http_msg_base(const http_msg_base& other) = delete;
    http_msg_base(http_msg_base&& other);
    virtual ~http_msg_base();

    http_msg_base& operator=(http_msg_base&& other);

    void pre_parse();
    void post_parse();

    virtual bool parse(buffer& b) = 0;

    std::size_t content_length() const;

    virtual std::string header_data() const = 0;
    const nx::data& data() const;

    std::string& h(const std::string& name);
    const std::string& h(const std::string& name) const;
    bool has(const header& h) const;
    bool has(const std::string& name) const;

    http_msg_base& operator<<(const header& h);
    http_msg_base& operator<<(const headers& h);
    http_msg_base& operator<<(const json& js);
    http_msg_base& operator<<(const jsonv::value& v);
    http_msg_base& operator<<(const file& f);

    template <typename T>
    http_msg_base& operator<<(const T& v)
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

template <typename Derived>
class http_msg : public http_msg_base
{
public:
    using http_msg_base::http_msg_base;

    template <typename T>
    Derived& operator<<(const T& v)
    {
        http_msg_base::operator<<(v);

        return *static_cast<Derived* const>(this);
    }
};

template <
    typename Socket,
    typename = std::enable_if_t<
        std::is_base_of<socket_base, Socket>::value
    >
>
Socket&
operator<<(Socket& s, const http_msg_base& m)
{
    s
        << m.header_data()
        << m.data()
        ;

    return s;
}

} // namespace nx

#endif // __NX_HTTP_MSG_H__
