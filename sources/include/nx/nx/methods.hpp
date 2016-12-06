#ifndef __NX_METHODS_H__
#define __NX_METHODS_H__

#include <string>

#include <nx/endpoint.hpp>

namespace nx {

const std::string get_method = "GET";
const std::string put_method = "PUT";
const std::string post_method = "POST";
const std::string delete_method = "DELETE";
const std::string options_method = "OPTIONS";

struct method
{
    const std::string& str;
};

const method GET = { get_method };
const method PUT = { put_method };
const method POST = { post_method };
const method DELETE = { delete_method };
const method OPTIONS = { options_method };

} // namespace nx

#endif // __NX_METHODS_H__
