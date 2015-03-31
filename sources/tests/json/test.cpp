#define BOOST_TEST_MODULE json

#include <iostream>

#include <nx/unit_test.hpp>

#include <nx/nx.hpp>
#include <nx/utils.hpp>

struct person {
    std::string name;
    std::size_t age;
};

auto person_fmt =
    jsonv::formats_builder()
    .type<person>()
        .member("name", &person::name)
        .member("age", &person::age)
    .check_references(jsonv::formats::defaults())
    ;

BOOST_AUTO_TEST_CASE(httpd_json)
{
    using namespace nx;

    const std::size_t deadline_count = 2;

    nx::timer deadlines[deadline_count];
    nx::cond_var cvs[deadline_count];

    for (std::size_t i = 0; i < deadline_count; i++) {
        deadlines[i](10.0) = [&](nx::timer& t, int events) {
            t.stop();
            cvs[i].notify();
        };

        deadlines[i].start();
    }

    add_json_format<person>(person_fmt);

    auto ep = endpoint("127.0.0.1");

    person p{ "John Doe", 42 };

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

        auto id = nx::to_num<std::size_t>(req.a("id"));
        person tp;

        json(data) >> tp;

        put_request_ok = id == 42 && tp.name == p.name && tp.age == p.age;
    };

    auto sep = hd(ep);

    httpc hc;

    bool got_get_reply = false;
    bool got_put_reply = false;
    bool get_reply_ok = false;
    bool put_reply_ok = false;

    hc(GET, sep) / "hello" = [&](const reply& rep, buffer& data) {
        got_get_reply = true;
        person tp;

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
    BOOST_CHECK_MESSAGE(got_put_reply, "httpc got GET reply");
    BOOST_CHECK_MESSAGE(get_reply_ok, "httpc got correct GET reply");
    BOOST_CHECK_MESSAGE(put_reply_ok, "httpc got correct PUT reply");
}
