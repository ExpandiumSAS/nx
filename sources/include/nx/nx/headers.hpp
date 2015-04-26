#ifndef __NX_HEADERS_H__
#define __NX_HEADERS_H__

#include <string>

#include <nx/config.h>
#include <nx/attributes.hpp>

namespace nx {

const std::string Content_Type = "Content-Type";
const std::string content_type = "content-type";
const std::string Content_Length = "Content-Length";
const std::string content_length = "content-length";
const std::string Content_Disposition = "Content-Disposition";
const std::string content_disposition = "content-disposition";
const std::string Location = "Location";
const std::string location = "location";
const std::string Upgrade = "Upgrade";
const std::string upgrade = "upgrade";
const std::string Connection = "Connection";
const std::string connection = "connection";
const std::string Sec_WebSocket_Key = "Sec-WebSocket-Key";
const std::string sec_websocket_key = "sec-websocket-key";
const std::string Sec_WebSocket_Protocol = "Sec-WebSocket-Protocol";
const std::string sec_websocket_protocol = "sec-websocket-protocol";
const std::string Sec_WebSocket_Version = "Sec-WebSocket-Version";
const std::string sec_websocket_version = "sec-websocket-version";
const std::string Sec_WebSocket_Accept = "Sec-WebSocket-Accept";
const std::string sec_websocket_accept = "sec-websocket-accept";

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

const header text_plain = { Content_Type, "text/plain" };
const header application_json = { Content_Type, "application/json" };
const header upgrade_websocket = { Upgrade, "websocket" };
const header connection_upgrade = { Connection, Upgrade };

} // namespace nx

#endif // __NX_HEADERS_H__
