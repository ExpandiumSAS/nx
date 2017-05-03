#define BOOST_TEST_MODULE local_json

#include <iostream>

#include <nx/unit_test.hpp>

#include <nx/nx.hpp>
#include <nx/utils.hpp>

#include <test/person.hpp>

BOOST_AUTO_TEST_CASE(httpd_json)
{
    using namespace nx;

    const std::size_t deadline_count = 2;

    nx::timer deadlines[deadline_count];
    nx::cond_var cvs[deadline_count];

    for (std::size_t i = 0; i < deadline_count; i++) {
        deadlines[i](10) = [&cvs,i](nx::timer& t) {
            t.stop();
            cvs[i].notify();
        };

        deadlines[i].start();
    }

    add_json_format<test::person>(test::person_fmt);

    auto ep = make_endpoint_local("/tmp/nx");

    test::person p{ "some id", "John Doe", 42 };

    httpd hd;

    bool got_get_request = false;
    bool got_put_request = false;
    bool put_request_ok = false;

    hd(GET) / "hello" = [&](const request& req, buffer& data, reply& rep) {
        got_get_request = true;
        rep << json(p);
    };

    hd(PUT) / "persons" / ":id" = [&](const request& req, buffer& data, reply& rep) {
        got_put_request = true;

        const auto& id = req.a("id");
        test::person tp;

        json(data) >> tp;

        put_request_ok = id == "some id" && tp.name == p.name && tp.age == p.age;
    };

    auto sep = hd(ep);

    httpc hc;

    bool got_get_reply = false;
    bool got_put_reply = false;
    bool get_reply_ok = false;
    bool put_reply_ok = false;

    hc(GET, sep) / "hello" = [&](const reply& rep, buffer& data) {
        got_get_reply = true;
        test::person tp;

        json(data) >> tp;

        get_reply_ok = tp.name == p.name && tp.age == p.age;

        deadlines[0].stop();
        cvs[0].notify();
    };

    hc(PUT, sep)
        / "persons/42"
        << json(p)
        = [&](const reply& rep, buffer& data) {
            got_put_reply = true;
            put_reply_ok = true;

            deadlines[1].stop();
            cvs[1].notify();
        };

    for (std::size_t i = 0; i < deadline_count; i++) {
        cvs[i].wait();
    }

    nx::stop();

    BOOST_CHECK_MESSAGE(got_get_request, "httpd got GET request");
    BOOST_CHECK_MESSAGE(got_put_request, "httpd got PUT request");
    BOOST_CHECK_MESSAGE(got_get_reply, "httpc got GET reply");
    BOOST_CHECK_MESSAGE(got_put_reply, "httpc got PUT reply");
    BOOST_CHECK_MESSAGE(get_reply_ok, "httpc got correct GET reply");
    BOOST_CHECK_MESSAGE(put_reply_ok, "httpc got correct PUT reply");
}
