#include <random>
#include <iterator>
#include <algorithm>

#include <nx/ws.hpp>
#include <nx/sha1.hpp>
#include <nx/basen.hpp>

namespace nx {

// Unique value from RFC 6455
const std::string ws_guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

ws::ws()
: base_type(),
parsed_(false)
{}

ws::ws(int fh)
: base_type(fh),
parsed_(false)
{}

ws::ws(ws&& other)
: base_type(std::forward<base_type>(other))
{ *this = std::move(other); }

ws::~ws()
{}

ws&
ws::operator=(ws&& other)
{
    base_type::operator=(std::forward<base_type>(other));
    parsed_ = other.parsed_;
    req_ = std::move(other.req_);
    rep_ = std::move(other.rep_);

    return *this;
}

bool
ws::request_parsed()
{
    if (!parsed_) {
        parsed_ = req_.parse(rbuf());
    }

    return parsed_;
}

bool
ws::reply_parsed()
{
    if (!parsed_) {
        parsed_ = rep_.parse(rbuf());
    }

    return parsed_;
}

void
ws::process_request()
{
    try {
        if (!request_parsed()) {
            // Wait until request is complete
            return;
        }

        // All data arrived, call upper handler
        server_handshake();
    } catch (const http_status& s) {
        rep_ << s;
    } catch (const std::exception& e) {
        std::cout << "BadRequest by " << e.what() << std::endl;
        rep_ << BadRequest(e);
    }

    *this << rep_.content();
}

std::string
ws::server_challenge() const
{
    nx::SHA1 s;

    s.update(req_.h(Sec_WebSocket_Key));
    std::string csum = s.final() + ws_guid;

    std::string c;

    bn::encode_b64(csum.begin(), csum.end(), std::back_inserter(c));

    return c;
}

void
ws::server_handshake()
{
    bool valid_request =
        req_ == GET
        &&
        req_.has(upgrade_websocket)
        &&
        req_.has(connection_upgrade)
        &&
        req_.has(Sec_WebSocket_Key)
        &&
        req_.has(Sec_WebSocket_Version)
        ;

    if (!valid_request) {
        throw BadRequest;
    }

    rep_
        << SwitchingProtocols
        << upgrade_websocket
        << connection_upgrade
        << header{ Sec_WebSocket_Accept, server_challenge() }
        ;
}

void
ws::process_reply()
{
    try {
        if (!reply_parsed()) {
            // Wait until response is complete
            return;
        }
    } catch (const http_status& s) {
        rep_ << s;
    } catch (const std::exception& e) {
        rep_ << BadResponse(e);
    }

    // All data arrived, call upper handler
    push_close();
}

void
ws::send_request()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist(0, 9);
    std::vector<uint8_t> buffer(16);

    std::generate(
        buffer.begin(), buffer.end(),
        [&]() { return dist(gen); }
    );

    std::string key;

    bn::encode_b64(buffer.begin(), buffer.end(), std::back_inserter(key));

    req_
        << header("Host", local().str())
        << upgrade_websocket
        << connection_upgrade
        << header{ Sec_WebSocket_Key, key }
        ;

    *this << req_.content();
}

} // namespace nx

