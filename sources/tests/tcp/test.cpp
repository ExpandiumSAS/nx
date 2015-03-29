#define BOOST_TEST_MODULE tcp

#include <iostream>
#include <string>

#include <nx/unit_test.hpp>

#include <nx/nx.hpp>

BOOST_AUTO_TEST_CASE(tcp_client_server)
{
    using namespace nx::tags;

    std::string msg("a message");

    nx::tcp s;

    bool got_new_connection = false;
    bool got_msg = false;

    auto sep = s.serve(
        nx::endpoint("127.0.0.1", 0),
        [&](nx::tcp& t) {
            got_new_connection = true;
        },
        [&](nx::tcp& t, nx::buffer& buf) {
            std::string str;

            t >> str;

            got_msg = (str == msg);
            std::reverse(str.begin(), str.end());

            t << str;
        }
    );

    nx::tcp c;
    nx::cond_var cv;

    bool connected = false;
    bool got_reply = false;
    bool disconnected = false;

    c.connect(
        sep,
        [&](nx::tcp& t) {
            connected = true;
            t << msg;
        }
    );

    c[on_read] = [&](nx::tcp& t, nx::buffer& b) {
        std::string str;
        t >> str;

        std::reverse(str.begin(), str.end());
        got_reply = (str == msg);
    };

    c[on_eof] = [&](nx::tcp& t) {
        disconnected = true;
    };

    cv.wait();

    auto fh = s.fh();

    std::cout << "END " << fh << std::endl;
}
