#ifndef __NX_HEADERS_H__
#define __NX_HEADERS_H__

#include <string>

#include <nx/config.h>
#include <nx/attributes.hpp>

namespace nx {

const std::string content_type = "Content-Type";
const std::string content_type_lc = "content-type";
const std::string content_length = "Content-Length";
const std::string content_length_lc = "content-length";
const std::string content_disposition = "Content-Disposition";
const std::string content_disposition_lc = "content-disposition";

class NX_API headers : public attribute_map
{
public:
    using attribute_map::attribute_map;

    virtual std::ostream& operator()(std::ostream& oss) const;
};

struct header : public attribute_base
{
    using attribute_base::attribute_base;
};

const header text_plain = { content_type, "text/plain" };
const header application_json = { content_type, "application/json" };

} // namespace nx

#endif // __NX_HEADERS_H__
