#include <arpa/inet.h>
#include <uuid.h>

#include <random>
#include <iterator>
#include <algorithm>

#include <nx/local_ws.hpp>
#include <nx/sha1.hpp>
#include <nx/base64.hpp>
#include <nx/endian.hpp>

namespace nx {

// Unique value from RFC 6455
const std::string ws_guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

const std::size_t ws_max_len = 10 * 1024 * 1024;

void
local_ws::start()
{
    (*this)[tags::on_read] = [](local_ws& w) { 
        w.process_frames(); 
    };
    (*this)[tags::on_close] = [](local_ws& w) {
        w.process_close();
    };

    base_type::start();
    if (connect_cb_) {
        connect_cb_(context(self()));
    } 
}

void
local_ws::finish(uint16_t code)
{
    send_close_frame(code);
    close();
}


bool
local_ws::parse_frame(ws_frame& f)
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
        hlen = 2;
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
        uint32_t mask = le32toh(*((uint32_t*) b.data()));
        auto mask_data = (uint8_t*) &mask;
        b.erase(b.begin(), b.begin() + 4);

        f.payload.resize(b.size());

        for (std::size_t i = 0; i < b.size(); i++) {
            f.payload[i] = b[i] ^ mask_data[i % 4];
        }
    } else {
        f.payload << b;
    }

    b.clear();
    return true;
}

void
local_ws::process_frames()
{
    ws_frame f;

    while (parse_frame(f)) {
        switch(f.opcode) {
            case WS_OP_CONTINUATION_FRAME:
                // TODO continuation frame
            break;

            case WS_OP_PING:
                send_ping_pong_frame(false);
            break;

            case WS_OP_TEXT_FRAME:
            case WS_OP_BINARY_FRAME:
                message_cb_(context(self()), f.payload);
            break;

            case WS_OP_CLOSE: {
                uint16_t code = ((uint16_t)f.payload[0] << 8) | (f.payload[1] & 0xFF); 
                finish(code);
            }
            break;
        }
    }
}

void 
local_ws::process_close()
{
    if (finish_cb_) {
        finish_cb_(context(self()));
    }
}

// void
// ws::send_request()
// {
//     std::random_device rd;
//     std::mt19937 gen(rd());
//     std::uniform_int_distribution<uint8_t> dist(0, 9);
//     std::vector<uint8_t> buffer(16);

//     std::generate(
//         buffer.begin(), buffer.end(),
//         [&]() { return dist(gen); }
//     );

//     std::string key;

//     bn::encode_b64(buffer.begin(), buffer.end(), std::back_inserter(key));

//     req_
//         << header("Host", local_str())
//         << upgrade_websocket
//         << connection_upgrade
//         << header{ Sec_WebSocket_Key, key }
//         ;

//     // TOFIX: *this << req_.content();
//     *this << std::move(req_);
// }

void
local_ws::set_callbacks(const ws_connection& w)
{
    connect_cb_ = w.connect_cb;
    message_cb_ = w.message_cb;
    finish_cb_ = w.finish_cb;
}

std::string 
local_ws::uid()
{ return uid_; }

const std::string& 
local_ws::uid() const
{ return uid_; }

void 
local_ws::send_close_frame(uint16_t code)
{
    buffer frame(4);
    frame[0] = 0b10001000;
    frame[1] = 2;
    frame[2] = (uint8_t)(code >> 8) & 0xFF;
    frame[3] = (uint8_t)(code & 0xFF);
    (*this) << std::move(frame);
}


void 
local_ws::send_ping_pong_frame(bool ping)
{
    buffer frame(2);
    frame[0] = (ping) ? 0b10001001 : 0b10001010;
    frame[1] = 0;
    (*this) << std::move(frame);
}

std::string
local_ws::server_challenge(const request& req)
{
    nx::SHA1 s;

    std::string csum = req.h(Sec_WebSocket_Key) + ws_guid;
    s.update(csum);

    auto chash = s.digest();
    std::string challenge = base64::encode(chash.begin(), chash.end());

    return challenge;
}

void
local_ws::server_handshake(const request& req, reply& rep)
{
    bool valid_request =
        req == GET
        &&
        req.has(upgrade_websocket)
        &&
        req.has(connection_upgrade)
        &&
        req.has(Sec_WebSocket_Key)
        &&
        req.has(Sec_WebSocket_Version)
        ;

    if (!valid_request) {
        throw BadRequest;
    }

    rep
        << SwitchingProtocols
        << upgrade_websocket
        << connection_upgrade
        << header{ Sec_WebSocket_Accept, server_challenge(req) }
        ;
}

ws_ptr 
local_ws::self()
{ return std::static_pointer_cast<ws>(shared_from_this()); }

std::string 
local_ws::make_uid()
{
     static const char* const lut = "0123456789ABCDEF";

    uuid_t uuid;

    uuid_generate(uuid);

    std::string id;

    id.reserve(2 * sizeof(uuid_t));

    for (const auto& c : uuid) {
        id.push_back(lut[c >> 4]);
        id.push_back(lut[c & 15]);
    }

    return id;
}

} // namespace nx
