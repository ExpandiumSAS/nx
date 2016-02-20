#define BOOST_TEST_MODULE send_file

#include <iostream>

#include <nx/unit_test.hpp>
#include <nx/system_config.hpp>

#include <nx/nx.hpp>
#include <nx/utils.hpp>

BOOST_AUTO_TEST_CASE(send_file)
{
    using namespace nx;

    nx::timer deadline;
    nx::cond_var cv;

    nx::system_config sc;
    auto lorem_file = cxxu::cat_file(sc.test_data_dir(), "testfile.txt");
    auto lorem_data = nx::slurp_file(lorem_file);

    deadline(5) = [&](nx::timer& t) {
        t.stop();
        cv.notify();
    };

    deadline.start();

    auto ep = make_endpoint("127.0.0.1");

    httpd hd;

    bool got_request = false;

    hd(GET) / "lorem" = [&](const request& req, buffer& data, reply& rep) {
        got_request = true;
        rep
            << text_plain
            << nx::file{ lorem_file }
            ;
    };

    auto sep = hd(ep);

    httpc hc;

    bool got_reply = false;
    bool reply_ok = false;

    hc(GET, sep) / "lorem" = [&](const reply& rep, buffer& data) {
        got_reply = true;

        reply_ok = rep && data == lorem_data;

        deadline.stop();
        cv.notify();
    };

    cv.wait();
    nx::stop();

    BOOST_CHECK_MESSAGE(got_request, "httpd got a request");
    BOOST_CHECK_MESSAGE(got_reply, "httpc got a reply");
    BOOST_CHECK_MESSAGE(reply_ok, "httpc got correct reply");
}
