#ifndef __NX_JSON_COLLECTION_H__
#define __NX_JSON_COLLECTION_H__

#include <nx/httpd.hpp>
#include <nx/json.hpp>

namespace nx {

class NX_API json_collection
{
public:
    json_collection(httpd& hd);
};

} // namespace nx

#endif // __NX_JSON_COLLECTION_H__
