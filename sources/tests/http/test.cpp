#define BOOST_TEST_MODULE nx_http

#include <iostream>

#include <nx/unit_test.hpp>

#include <nx/nx.hpp>
#include <nx/utils.hpp>

BOOST_AUTO_TEST_CASE(nx_escape)
{
    std::string s("some text with % chars to escape àéiôu");

    BOOST_CHECK_MESSAGE(
        nx::unescape(nx::escape(s)) == s,
        "escape / unescape"
    );

    BOOST_CHECK_NO_THROW(
        nx::uri("http://server/some/path/to/res?var1=a&pouet=toto")
    );

    BOOST_CHECK_NO_THROW(
        nx::uri("/some/other/path?val=a&foo=bar&junk&no_val=")
    );

    BOOST_CHECK_THROW(
        nx::uri("some junk text"),
        std::runtime_error
    );
}

BOOST_AUTO_TEST_CASE(nx_httpd)
{
    using namespace nx;
    auto ep = endpoint("127.0.0.1");
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

    auto sep = hd(ep, "server");

    httpc hc;

    bool got_reply = false;
    bool reply_ok = false;

    hc(GET, sep) / "hello" = [&](const reply& rep, buffer& data) {
        got_reply = true;

        reply_ok = rep && data == hello_world;

        nx::stop();
    };

    nx::run();

    BOOST_CHECK_MESSAGE(got_request, "httpd got a request");
    BOOST_CHECK_MESSAGE(got_reply, "httpc got a reply");
    BOOST_CHECK_MESSAGE(reply_ok, "httpc got correct reply");
}

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

void test_done()
{
    static const std::size_t stop_count = 2;
    static std::size_t count = 0;

    if (++count == stop_count) {
        nx::stop();
    }
}

BOOST_AUTO_TEST_CASE(nx_httpd_json)
{
    using namespace nx;

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

    auto sep = hd(ep, "server");

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

        test_done();
    };

    hc(PUT, sep)
        / "persons/42"
        << json(p)
        = [&](const reply& rep, buffer& data) {
            got_put_reply = true;
            put_reply_ok = true;

            test_done();
        };

    nx::run();

    BOOST_CHECK_MESSAGE(got_get_request, "httpd got GET request");
    BOOST_CHECK_MESSAGE(got_put_request, "httpd got PUT request");
    BOOST_CHECK_MESSAGE(got_get_reply, "httpc got GET reply");
    BOOST_CHECK_MESSAGE(got_put_reply, "httpc got GET reply");
    BOOST_CHECK_MESSAGE(get_reply_ok, "httpc got correct GET reply");
    BOOST_CHECK_MESSAGE(put_reply_ok, "httpc got correct PUT reply");
}
