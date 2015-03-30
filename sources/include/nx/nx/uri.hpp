#ifndef __NX_URI_H__
#define __NX_URI_H__

#include <stdint.h>

#include <string>

#include <nx/config.h>
#include <nx/attributes.hpp>

namespace nx {

class NX_API uri
{
public:
    uri(const std::string& u);

    std::string& scheme();
    const std::string& scheme() const;
    std::string& host();
    const std::string& host() const;
    uint16_t& port();
    const uint16_t port() const;
    std::string& path();
    const std::string& path() const;
    attributes& a();
    const attributes& a() const;

private:
    void parse(const std::string& u);

    std::string scheme_;
    std::string host_;
    uint16_t port_;
    std::string path_;
    attributes a_;
};

} // namespace nx

#endif // __NX_URI_H__
