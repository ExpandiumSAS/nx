#define BOOST_TEST_MODULE utils

#include <iostream>

#include <nx/unit_test.hpp>

#include <nx/nx.hpp>
#include <nx/utils.hpp>

BOOST_AUTO_TEST_CASE(escape)
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
        nx::http_status
    );
}
