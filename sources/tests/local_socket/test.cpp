#define BOOST_TEST_MODULE local_socket

#include <iostream>
#include <string>
#include <atomic>

#include <nx/unit_test.hpp>

#include <nx/nx.hpp>

BOOST_AUTO_TEST_CASE(local_socket_client_server)
{
    using namespace nx::tags;

    nx::timer deadline;
    nx::cond_var cv;

    deadline(5) = [&](nx::timer& t) {
        t.stop();
        cv.notify();
    };

    deadline.start();

    std::string msg("a message");

    std::atomic_bool got_new_connection{ false };
    std::atomic_bool got_msg{ false };

    auto endpoint = nx::serve<nx::local_socket>(
        nx::make_endpoint_local("/tmp/nx"),
        [&](nx::local_socket& t) {
            got_new_connection = true;
        },
        [&](nx::local_socket& t) {
            std::string str;

            t >> str;

            got_msg = (str == msg);
            std::reverse(str.begin(), str.end());

            t << str;
            t.stop();
        }
    );

    std::atomic_bool connected{ false };
    std::atomic_bool got_reply{ false };
    std::atomic_bool got_correct_reply{ false };
    std::atomic_bool disconnected{ false };

    auto& c = nx::connect<nx::local_socket>(
        endpoint,
        [&](nx::local_socket& t) {
            connected = true;

            t[on_close] = [&](nx::local_socket& t) {
                disconnected = true;
            };

            t << msg;
        }
    );

    c[on_read] = [&](nx::local_socket& t) {
        got_reply = true;

        std::string str;
        t >> str;

        std::reverse(str.begin(), str.end());
        got_correct_reply = (str == msg);

        deadline.stop();
        cv.notify();
    };

    cv.wait();
    nx::stop();

    BOOST_CHECK_MESSAGE(got_new_connection, "server got a connection");
    BOOST_CHECK_MESSAGE(got_msg, "server got correct message");
    BOOST_CHECK_MESSAGE(connected, "client connected to server");
    BOOST_CHECK_MESSAGE(got_reply, "client got a reply");
    BOOST_CHECK_MESSAGE(got_correct_reply, "client got correct reply");
    BOOST_CHECK_MESSAGE(disconnected, "client was disconnected");
}
