#include <arpa/inet.h>

#include <random>
#include <iterator>
#include <algorithm>

#include <nx/ws.hpp>
#include <nx/sha1.hpp>
#include <nx/basen.hpp>
#include <nx/endian.hpp>

namespace nx {

// Unique value from RFC 6455
const std::string ws_guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

const std::size_t ws_max_len = 10 * 1024 * 1024;

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

void
ws::finish(int code)
{}

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

bool
ws::parse_frame(ws_frame& f)
{
    auto& b = rbuf();
    auto data = b.data();

    if (b.size() < 2) {
        return false;
    }

    auto first = data[0];
    auto second = data[1];

    f.fin = ((first & 0b10000000) == 0b10000000);

    f.rsv1 = ((first & 0b01000000) == 0b01000000);
    f.rsv2 = ((first & 0b00100000) == 0b00100000);
    f.rsv3 = ((first & 0b00010000) == 0b00010000);

    f.opcode = first & 0b00001111;

    std::size_t hlen = 2;
    std::size_t len = (second & 0b01111111);

    if (len < 126) {
        std::cout << "small payload: " << len << std::endl;
    } else if (len == 126) {
        if (b.size() <= 4) {
            // Not enough header data
            return false;
        }

        hlen = 4;
        len = (std::size_t) be16toh(*((uint16_t*) (data + 2)));

        std::cout << "extended 16-bit payload: " << len << std::endl;
    } else if (len == 127) {
        if (b.size() <= 10) {
            // Not enough header data
            return false;
        }

        hlen = 10;
        len = (std::size_t) be64toh(*((uint64_t*) (data + 2)));

        std::cout << "extended 64-bit payload: " << len << std::endl;
    }

    if (len > ws_max_len) {
        finish(1009);

        return false;
    }

    // Check if whole packet has arrived
    bool masked = second & 0b10000000;

    if (masked) {
        len += 4;
    }

    if (b.size() < hlen + len) {
        return false;
    }

    // Remove header bytes
    b.erase(b.begin(), b.begin() + hlen);

    // Payload
    if (masked) {
        // Extract mask
        uint32_t mask = be32toh(*((uint32_t*) b.data()));
        auto mask_data = (uint8_t*) &mask;
        b.erase(b.begin(), b.begin() + 2);

        f.payload.resize(b.size());

        for (std::size_t i = 0; i < b.size(); i++) {
            f.payload[i] = b[i] ^ mask_data[i % 4];
        }
    } else {
        f.payload << b;
    }

    return true;
}

void
ws::process_frames()
{
    ws_frame f;

    while (parse_frame(f)) {

    }
}

void
ws::process_request()
{
    try {
        if (!request_parsed()) {
            // Wait until request is complete
            return;
        }

        // Request complete, perform handshake
        server_handshake();
    } catch (const http_status& s) {
        rep_ << s;
    } catch (const std::exception& e) {
        std::cout << "BadRequest by " << e.what() << std::endl;
        rep_ << BadRequest(e);
    }

    *this << rep_.content();

    // From now on, process websocket frames
    (*this)[tags::on_read] = [](ws& w) { w.process_frames(); };
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

