#ifndef __NX_REPLY_H__
#define __NX_REPLY_H__

#include <ostream>
#include <sstream>
#include <string>
#include <memory>

#include <nx/picohttpparser.h>

#include <nx/config.h>
#include <nx/buffer.hpp>
#include <nx/headers.hpp>
#include <nx/http_status.hpp>
#include <nx/json.hpp>
#include <nx/handlers.hpp>

namespace nx {

class NX_API reply
{
public:
    reply();
    reply(const reply& other) = delete;
    reply(reply&& other);
    virtual ~reply();

    reply& operator=(reply&& other);

    operator bool() const;

    bool parse(buffer& b);

    std::size_t content_length() const;

    const http_status& code() const;

    std::string& h(const std::string& name);
    const std::string& h(const std::string& name) const;

    std::string content() const;

    void postpone();
    bool postponed();

    void done();

    bool operator==(const http_status& s) const;
    bool operator!=(const http_status& s) const;

    reply& operator<<(const http_status& s);
    reply& operator<<(const header& h);
    reply& operator<<(const jsonv::value& v);

    template <typename T>
    reply& operator<<(const T& v)
    {
        data_ << v;

        return *this;
    }

protected:
    friend class http;

    void_cb& on_done();

private:
    using raw_headers_ptr = std::unique_ptr<phr_header[]>;
    static const std::size_t max_headers = 128;

    raw_headers_ptr raw_headers_ptr_;
    headers headers_;
    http_status status_;
    std::size_t content_length_;
    std::ostringstream data_;
    bool postponed_;
    void_cb done_cb_;

    int minor_version_;
    int raw_status_;
    const char* raw_msg_;
    std::size_t raw_msg_len_;
    std::size_t num_headers_;
    std::size_t prev_buf_len_ = 0;
};

inline
std::ostream&
operator<<(std::ostream& os, const reply& r)
{
    os << r.content();
    return os;
}

} // namespace nx

#endif // __NX_REQUEST_H__
