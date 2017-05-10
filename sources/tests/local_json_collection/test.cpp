#define BOOST_TEST_MODULE local_json_collection

#include <iostream>

#include <nx/unit_test.hpp>

#include <nx/nx.hpp>
#include <nx/utils.hpp>

#include <test/person.hpp>

// Configure a deadline timer for each test
const std::size_t test_count = 4;

nx::timer deadline;
nx::cond_var cv(test_count);

BOOST_AUTO_TEST_CASE(local_httpd_json_collection)
{
    using namespace nx;

    deadline(10) = [&](nx::timer& t) {
        t.stop();
        cv.notify_all();
    };

    deadline.start();

    using collection_type = make_json_collection<>::type;

    collection_type coll("persons");

    httpd hd;

    // Register collection in httpd
    hd << coll;

    // Start server
    auto sep = hd(make_endpoint("/tmp/nx"));

    httpc hc;

    bool item_not_found = false;

    hc(GET, sep) / "persons/1234" = [&](const reply& rep, buffer& data) {
        item_not_found = (rep == NotFound);

        cv.notify();
    };

    bool got_collection = false;
    bool empty_collection = false;

    hc(GET, sep) / "persons" = [&](const reply& rep, buffer& data) {
        got_collection = (rep == OK);

        json persons(data);

        empty_collection = persons.value().empty();

        cv.notify();
    };

    bool item_created = false;
    bool item_has_id = false;
    bool item_with_id_found = false;
    bool item_is_correct = false;

    hc(POST, sep)
        / "persons"
        << jsonv::object({
            { "id", "42" },
            { "name", "Bart Simpson" },
            { "age", 15 }
        })
        = [&](const reply& rep, buffer& data) {
            item_created = (rep == Created);

            auto parts = split("/", rep.h(location));

            if (!parts.empty()) {
                item_has_id = true;

                hc(GET, sep) / "persons" / parts.back() =
                    [&](const reply& rep, buffer& data) {
                        item_with_id_found = (rep == OK);

                        json person(data);
                        const auto& p = person.value();

                        item_is_correct = (
                            p.at("id").as_string() != "42"
                            &&
                            p.at("name").as_string() == "Bart Simpson"
                            &&
                            p.at("age").as_integer() == 15
                        );

                        cv.notify();
                    };
                }

                cv.notify();
        };

    cv.wait();
    nx::stop();

    BOOST_CHECK_MESSAGE(
        item_not_found,
        "non-existent item was not found"
    );
    BOOST_CHECK_MESSAGE(
        got_collection,
        "got a collection"
    );
    BOOST_CHECK_MESSAGE(
        empty_collection,
        "intial collection is empty"
    );
    BOOST_CHECK_MESSAGE(
        item_created,
        "a new item was created"
    );
    BOOST_CHECK_MESSAGE(
        item_has_id,
        "new item has an id"
    );
    BOOST_CHECK_MESSAGE(
        item_with_id_found,
        "new item with id was found"
    );
    BOOST_CHECK_MESSAGE(
        item_is_correct,
        "new item is correct"
    );
}
