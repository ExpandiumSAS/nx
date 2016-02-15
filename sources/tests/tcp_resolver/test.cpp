#define BOOST_TEST_MODULE tcp_resolver

#include <iostream>
#include <string>
#include <atomic>

#include <nx/unit_test.hpp>

#include <nx/nx.hpp>

BOOST_AUTO_TEST_CASE(tcp_resolver)
{
    using namespace nx::tags;

    nx::timer deadline;
    nx::cond_var cv;

    deadline(5) = [&](nx::timer& t) {
        t.stop();
        cv.notify();
    };

    deadline.start();

    std::atomic_bool got_new_connection{ false };

    nx::serve<nx::tcp>(
        nx::make_endpoint("127.0.0.1", 54321),
        [&](nx::tcp& t) {
            got_new_connection = true;
            t.stop();
        },
        [&](nx::tcp& t) {}
    );

    std::atomic_bool connected{ false };
    std::atomic_bool disconnected{ false };

    auto& c = nx::connect<nx::tcp>(
        "localhost", "54321",
        [&](nx::tcp& t) {
            connected = true;

            t[on_close] = [&](nx::tcp& t) {
                disconnected = true;
                deadline.stop();
                cv.notify();
            };
        }
    );

    cv.wait();
    nx::stop();

    BOOST_CHECK_MESSAGE(got_new_connection, "server got a connection");
    BOOST_CHECK_MESSAGE(connected, "client connected to server");
    BOOST_CHECK_MESSAGE(disconnected, "client was disconnected");
}
