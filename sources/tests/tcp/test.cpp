#define BOOST_TEST_MODULE tcp

#include <iostream>
#include <string>

#include <nx/unit_test.hpp>

#include <nx/nx.hpp>

BOOST_AUTO_TEST_CASE(tcp_client_server)
{
    using namespace nx::tags;
    nx::tcp s;

    auto sep = s.serve(
        nx::endpoint("127.0.0.1", 0),
        [](nx::tcp&& t) {
            std::cout
                << "new connection from "
                << t.remote()
                << std::endl;
        },
        [](nx::tcp& t, nx::buffer& buf) {
            std::cout
                << t.remote()
                << " sent: " << buf.size()
                << std::endl;
        }
    );

    std::cout << "s on " << sep << std::endl;

    nx::tcp c;
    nx::cond_var cv;

    c.connect(
        sep,
        [&](nx::tcp& t) {
            std::cout
                << t.local() << " connected to " << t.remote()
                << std::endl;
            cv.notify();
        }
    );

    cv.wait();

    std::cout << "END" << std::endl;
}
