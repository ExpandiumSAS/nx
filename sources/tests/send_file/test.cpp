#define BOOST_TEST_MODULE send_file

#include <iostream>

#include <nx/unit_test.hpp>

#include <nx/nx.hpp>
#include <nx/utils.hpp>

BOOST_AUTO_TEST_CASE(send_file)
{
    using namespace nx;

    nx::timer deadline;
    nx::cond_var cv;

    deadline(5) = [&](nx::timer& t) {
        t.stop();
        cv.notify();
    };

    deadline.start();

    auto ep = make_endpoint("127.0.0.1");
    const char* hello_world = "Hello, world!";

    httpd hd;

    bool got_request = false;

    hd(GET) / "hello" = [&](const request& req, buffer& data, reply& rep) {
        got_request = true;
        rep
            << text_plain
            << hello_world
            ;
    };

    auto sep = hd(ep);

    httpc hc;

    bool got_reply = false;
    bool reply_ok = false;

    hc(GET, sep) / "hello" = [&](const reply& rep, buffer& data) {
        got_reply = true;

        reply_ok = rep && data == hello_world;

        deadline.stop();
        cv.notify();
    };

    cv.wait();
    nx::stop();

    BOOST_CHECK_MESSAGE(got_request, "httpd got a request");
    BOOST_CHECK_MESSAGE(got_reply, "httpc got a reply");
    BOOST_CHECK_MESSAGE(reply_ok, "httpc got correct reply");
}
