#ifndef __NX_ERROR_CODE_H__
#define __NX_ERROR_CODE_H__

#include <string>
#include <ostream>

#include <nx/config.h>

namespace nx {

class NX_API error_code
{
public:
    error_code(int status);
    error_code(const char* msg, int status);
    error_code(const error_code&) = default;
    error_code(error_code&&) = default;

    int status() const;
    std::string what() const;

    error_code& operator=(const error_code&) = default;
    error_code& operator=(error_code&&) = default;
    error_code& operator=(int status);

    bool operator==(const error_code& other) const;
    bool operator==(int status) const;

    operator bool() const;

private:
    int status_;
    const char* what_;
    const char* msg_;
};

inline
std::ostream&
operator<<(std::ostream& os, const error_code& e)
{
    os << e.what();
    return os;
}

NX_API
void check_for_error(const char* what, int status);

NX_API
void check_for_error(const char* what, const error_code& e);

} // namespace nx

#endif // __NX_ERROR_CODE_H__
