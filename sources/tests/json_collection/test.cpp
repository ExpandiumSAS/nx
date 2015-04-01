#define BOOST_TEST_MODULE json_collection

#include <iostream>

#include <nx/unit_test.hpp>

#include <nx/nx.hpp>
#include <nx/utils.hpp>

#include "../json/person.hpp"

BOOST_AUTO_TEST_CASE(httpd_json_collection)
{
    using namespace nx;

    // Configure a deadline timer for each test
    const std::size_t deadline_count = 3;

    nx::timer deadlines[deadline_count];
    nx::cond_var cvs[deadline_count];

    for (std::size_t i = 0; i < deadline_count; i++) {
        deadlines[i](10.0) = [&](nx::timer& t, int events) {
            t.stop();
            cvs[i].notify();
        };

        deadlines[i].start();
    }

    using collection_type = json_collection<test::person>;
    using persons = collection_type::values_type;

    // Register test::person type
    add_json_format<test::person>(test::person_fmt);
    add_json_format<persons>(collection_type::format());

    collection_type coll("persons");

    httpd hd;

    // Register collection in httpd
    hd << coll;

    // Start server
    auto sep = hd(endpoint("127.0.0.1"));

    httpc hc;

    bool item_not_found = false;

    hc(GET, sep) / "persons/1234" = [&](const reply& rep, buffer& data) {
        item_not_found = (rep == not_found());

        deadlines[0].stop();
        cvs[0].notify();
    };

    bool got_collection = false;
    bool empty_collection = false;

    hc(GET, sep) / "persons" = [&](const reply& rep, buffer& data) {
        got_collection = (rep == OK);
        persons p;

        json(data) >> p;

        empty_collection = p.empty();

        deadlines[1].stop();
        cvs[1].notify();
    };

    bool item_created = false;

    hc(POST, sep) / "persons" = [&](const reply& rep, buffer& data) {
        item_created = (rep == Created);

        std::cout << "Location: " << rep.h(location_lc) << std::endl;

        deadlines[2].stop();
        cvs[2].notify();
    };

    for (std::size_t i = 0; i < deadline_count; i++) {
        cvs[i].wait();
    }

    nx::stop();
}
