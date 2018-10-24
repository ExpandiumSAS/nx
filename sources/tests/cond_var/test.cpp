#define BOOST_TEST_MODULE cond_var

#include <iostream>

#include <nx/unit_test.hpp>

#include <nx/nx.hpp>
#include <nx/utils.hpp>

BOOST_AUTO_TEST_CASE(cond_var_timeout)
{
    using namespace nx;

    bool got_request = false;
    bool got_reply = false;
    bool reply_ok = false;
    bool got_timeout_reply = false;

    auto ep = make_endpoint("127.0.0.1");
    const char* hello_world = "Hello, world!";

    // Server side : the response is generated after 2s delay
    httpd hd;
    hd(GET) / "hello" = [&](const request& req, buffer& data, reply& rep) {
        //std::cout << "Got a request" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        got_request = true;
        //std::cout << "Send a response" << std::endl;
        rep
            << text_plain
            << hello_world
            ;
    };

    auto sep = hd(ep);

    // Client side
    httpc hc;
    {
        // Create a smart_ptr on a condition variable
        auto cv = nx::new_cond_var();

        //std::cout << "Send a request" << std::endl;
        // Pass by value the cond_var, not by reference so the count use increase
        hc(GET, sep) / "hello" = [&, cv](const reply& rep, buffer& data) {
            //std::cout << "Get a response" << std::endl;
            // Check that cond_var is alway alive here and outside this lambda
            if (cv.use_count() == 1) {
                //std::cout << "too late..." << std::endl;
                // use_count shoud be 1, it means that the cond_var only
                // live inside this lamdba and we got the answer too late
                got_timeout_reply = true;
            } else {
                // use_count is 2 or more, the cond_var is alive outside this lambda.
                got_reply = true;

                reply_ok = (rep && data == hello_world);

                cv->notify();
            }
        };

        // Waiting for 1s before going on timeout
        bool wait_status = cv->wait_for(1000);
        BOOST_CHECK_MESSAGE(!wait_status, "wait_statut should be false due to timeout");
        //std::cout << "Stop waiting for response, status: " << wait_status << std::endl;
    }

    // Waiting for 4s to
    //std::cout << "Waiting 4 sec" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(4));

    //std::cout << "nx stop" << std::endl;
    nx::stop();

    BOOST_CHECK_MESSAGE(got_request, "httpd got a request");
    BOOST_CHECK_MESSAGE(!got_reply, "httpc got a reply");
    BOOST_CHECK_MESSAGE(!reply_ok, "httpc got correct reply");
    BOOST_CHECK_MESSAGE(got_timeout_reply, "httpc got a timeout reply");
}
