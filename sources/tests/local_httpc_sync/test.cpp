#define BOOST_TEST_MODULE local_httpc_sync

#include <nx/unit_test.hpp>

#include <nx/nx.hpp>
#include <nx/utils.hpp>

BOOST_AUTO_TEST_SUITE(local_http_sync_client);

BOOST_AUTO_TEST_CASE(local_http_sync_client_in_httpd_handle)
{
    using namespace nx;

    auto ep = make_endpoint_local("/tmp/nx");

    httpd hd;

    bool got_request = false;
    hd(GET) / "test_httpc_sync" = [&got_request](const request& req, buffer& data, reply& rep) {
        got_request = true;
    };

    // Listener
    auto sep = hd(ep);

    // Client Launcher
    // Client (wait for response)
    httpc_sync hcs;
    bool got_reply = false;
    hcs(GET, sep) / "test_httpc_sync" = [&got_reply](const reply& rep, buffer& data){
        got_reply = true;
    };

    BOOST_CHECK_MESSAGE(got_request, "httpd got request");
    BOOST_CHECK_MESSAGE(got_reply, "httpc_sync reply");

    nx::stop();
}

BOOST_AUTO_TEST_SUITE_END();
