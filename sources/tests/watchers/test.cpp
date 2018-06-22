#define BOOST_TEST_MODULE watchers

#include <iostream>

#include <nx/unit_test.hpp>

#include <nx/nx.hpp>

BOOST_AUTO_TEST_CASE(timer)
{
    nx::timer t;
    nx::cond_var cv;

    bool error_happened = false;

    t[nx::tags::on_error] = [&](nx::timer& t, const nx::error_code& ec) {
        error_happened = true;
    };

    bool got_timer = false;

    t(std::chrono::milliseconds(25)) = [&](nx::timer& t) {
        got_timer = true;
        t.stop();
        cv.notify();
    };

    t.start();

    cv.wait();

    BOOST_CHECK_MESSAGE(got_timer, "timer activated");
    BOOST_ASSERT(!error_happened);
}
