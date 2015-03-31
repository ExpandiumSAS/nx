#define BOOST_TEST_MODULE tcp

#include <iostream>
#include <string>

#include <nx/unit_test.hpp>

#include <nx/nx.hpp>

BOOST_AUTO_TEST_CASE(tcp_client_server)
{
    using namespace nx::tags;

    nx::timer deadline;
    nx::cond_var cv;

    deadline(5.0) = [&](nx::timer& t, int events) {
        t.stop();
        cv.notify();
    };

    deadline.start();

    std::string msg("a message");

    bool got_new_connection = false;
    bool got_msg = false;

    auto endpoint = nx::serve<nx::tcp>(
        nx::endpoint("127.0.0.1", 0),
        [&](nx::tcp& t) {
            got_new_connection = true;
        },
        [&](nx::tcp& t) {
            std::string str;

            t >> str;

            got_msg = (str == msg);
            std::reverse(str.begin(), str.end());

            t << str;
            t.push_close();
        }
    );

    bool connected = false;
    bool got_reply = false;
    bool disconnected = false;

    auto& c = nx::connect<nx::tcp>(
        endpoint,
        [&](nx::tcp& t) {
            connected = true;
            t << msg;
        }
    );

    c[on_read] = [&](nx::tcp& t) {
        std::string str;
        t >> str;

        std::reverse(str.begin(), str.end());
        got_reply = (str == msg);

        deadline.stop();
        cv.notify();
    };

    c[on_eof] = [&](nx::tcp& t) {
        disconnected = true;
    };

    cv.wait();

    BOOST_CHECK_MESSAGE(got_new_connection, "server got a connection");
    BOOST_CHECK_MESSAGE(got_msg, "server got correct message");
    BOOST_CHECK_MESSAGE(connected, "client connected to server");
    BOOST_CHECK_MESSAGE(got_reply, "client got correct reply");
    BOOST_CHECK_MESSAGE(disconnected, "client was disconnected");
}
