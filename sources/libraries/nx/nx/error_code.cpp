#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sstream>

#include <nx/error_code.hpp>

namespace nx {

const char* no_error = "no error";

inline
const char*
make_what(int status)
{ return status == 0 ? no_error : std::strerror(errno); }

error_code::error_code(int status)
: status_(status),
what_(make_what(status_)),
msg_(nullptr)
{}

error_code::error_code(const char* msg, int status)
: status_(status),
what_(make_what(status_)),
msg_(msg)
{}

int
error_code::status() const
{ return status_; }

std::string
error_code::what() const
{
    std::ostringstream oss;

    if (msg_ != nullptr) {
        oss << msg_ << ": ";
    }

    oss << what_ << " (" << status_ << ")";

    return oss.str();
}

error_code&
error_code::operator=(int status)
{
    status_ = status;
    what_ = make_what(status_);

    return *this;
}

bool
error_code::operator==(const error_code& other) const
{ return status_ == other.status_; }

bool
error_code::operator==(int status) const
{ return status_ == status; }

error_code::operator bool() const
{ return status_ != 0; }

void
check_for_error(const char* what, int status)
{
    if (status != 0) {
        std::ostringstream oss;

        oss
            << what << ": "
            << make_what(status)
            ;

        throw std::runtime_error(oss.str());
    }
}

void
check_for_error(const char* what, const error_code& e)
{ check_for_error(what, e.status()); }

} // namespace nx
