#define BOOST_TEST_MODULE ws

#include <iostream>

#include <nx/unit_test.hpp>

#include <nx/nx.hpp>
#include <nx/request.hpp>
#include <nx/ws.hpp>

BOOST_AUTO_TEST_CASE(ws_server)
{
    using namespace nx;

    std::string accept_value = "dGhlIHNhbXBsZSBub25jZQ==";
    request req;
    req << header{ Sec_WebSocket_Key, accept_value };
    BOOST_CHECK_EQUAL("s3pPLMBiTxaQ9kYGzzhZRbK+xOo=", ws<tcp_base>::server_challenge(req));

    // httpd hd;
    // hd(WS) / "echo" = ws_connection(
    //     [](auto&& ctx, const buffer& data) {
    //         std::string str(data.begin(), data.end());
    //         ctx << ws_text << "reponse: " << str;
    //     }
    // );

    // cond_var cv;

    // hd(GET) / "stop" = [&cv](const request& req, buffer& data, reply& rep) {
    //     rep << "stop";
    //     cv.notify();
    // };

    // auto ep = make_endpoint("0.0.0.0", 1234);
    // hd(ep);

    // cv.wait();
}