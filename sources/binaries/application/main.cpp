#include <iostream>

#include <nx/nx.hpp>

#include <cstdlib>
#include <cstring>
#include <vector>

using namespace nx;

#define MAX_LEN 1024

int main(int ac, char **av)
{
    if (ac <= 1) {
        return 1;
    }

    std::string name(av[1]);
    bool on = true;
    httpd srv;
    cond_var cv;
    // Register a GET handler
    srv(GET) / "HELLO_NAME" = [&](const request &req, buffer &data, reply &rep) {
        if (on)
        {
            rep
                << text_plain
                << "Hello, I am "
                << name
                << ".";
        }
    };

    srv(GET) / "BYE_NAME" = [&](const request &req, buffer &data, reply &rep) {
        if (on)
        {
            rep
                << text_plain
                << "Bye from "
                << name
                <<"!";
        }
    };

    // Bind and listen
    srv(make_endpoint("/tmp/nx_" + name));

    cv.wait();
    nx::stop();
}