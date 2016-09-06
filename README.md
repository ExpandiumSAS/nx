nx - Modern C++ HTTP library
============================

nx is a simple and modern C++ library allowing fast development of HTTP
servers and clients in just a few lines of code.

For example:

A simple "Hello World !" server:

~~~cpp
#include <nx/nx.hpp>

using namespace nx;

int main(int ac, char **av)
{
    httpd srv;

    // Register a GET handler
    srv(GET) / "Hello" = [&](const request& req, buffer& data, reply& rep) {
        rep
            << text_plain
            << "Hello, world!"
            ;
    };

    // Bind and listen
    srv(make_endpoint("127.0.0.1", 3000));

    // Do something else while server asynchronously handles request...
}
~~~

The corresponding client:

~~~cpp
#include <nx/nx.hpp>

using namespace nx;

int main(int ac, char **av)
{
    httpc cli;

    // Register a GET handler
    cli(
        GET,
        make_endpoint("127.0.0.1", 3000)
    ) / "Hello" = [&](const reply& rep, buffer& data) {
        if (rep) {
            std::cout
                << "server replied:\n"
                << data
                ;
        }
    };

    // Do something else...
}
~~~
