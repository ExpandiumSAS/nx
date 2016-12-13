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

void
ws::start()
{
    (*this)[tags::on_read] = [](ws& w) { w.process_frames(); };
}

void
ws::finish(uint16_t code)
{
    send_close_frame(code);
    finish_cb_(ctx_);
    close();
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
        switch(f.opcode) {
            case WS_OP_CONTINUATION_FRAME:
                // TODO continuation frame
            break;

            case WS_OP_PING:
                send_ping_pong_frame(false);
            break;

            case WS_OP_TEXT_FRAME:
            case WS_OP_BINARY_FRAME:
                message_cb_(ctx_, f.payload);
            break;

            case WS_OP_CLOSE: {
                uint16_t code = ((uint16_t)f.payload[0] << 8) | (f.payload[1] & 0xFF); 
                finish(code);
            }
            break;
        }
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
ws::send_close_frame(uint16_t code)
{
    buffer frame(4);
    frame[0] = 0b10001000;
    frame[1] = 2;
    frame[2] = (uint8_t)(code >> 8) & 0xFF;
    frame[3] = (uint8_t)(code & 0xFF);
    (*this) << std::move(frame);
}


void 
ws::send_ping_pong_frame(bool ping)
{
    buffer frame(2);
    frame[0] = (ping) ? 0b10001001 : 0b10001010;
    frame[1] = 0;
    (*this) << std::move(frame);
}

std::string
ws::server_challenge(const request& req)
{
    nx::SHA1 s;

    s.update(req.h(Sec_WebSocket_Key));
    std::string csum = s.final() + ws_guid;

    std::string c;

    bn::encode_b64(csum.begin(), csum.end(), std::back_inserter(c));

    return c;
}

void
ws::server_handshake(const request& req, reply& rep)
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

} // namespace nx

