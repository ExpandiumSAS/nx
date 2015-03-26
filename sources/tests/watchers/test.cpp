#define BOOST_TEST_MODULE watchers

#include <iostream>

#include <nx/unit_test.hpp>

#include <nx/nx.hpp>

BOOST_AUTO_TEST_CASE(timer)
{
    nx::timer t;
    nx::cond_var cv;

    nx::loop::get().start();

    bool got_timer = false;

    t(.25) = [&](nx::timer& t, int events) {
        got_timer = true;
        t.stop();
        cv.notify();
    };

    t.start();

    cv.wait();

    BOOST_CHECK_MESSAGE(
        got_timer,
        "timer activated"
    );
}
