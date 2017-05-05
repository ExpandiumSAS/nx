#include <iostream>

#include <nx/nx.hpp>

#include <cstdlib>
#include <cstring>
#include <vector>

using namespace nx;

#define MAX_LEN 1024

int main(int ac, char **av)
{
    bool on = true;
    httpd srv;
    cond_var cv;
    // Register a GET handler
    srv(GET) / "Hello" = [&](const request &req, buffer &data, reply &rep) {
        if (on)
        {
            rep
                << text_plain
                << "Hello, world!";
        }
    };

    srv(GET) / "Bye" = [&](const request &req, buffer &data, reply &rep) {
        if (on)
        {
            rep
                << text_plain
                << "Bye world!";
        }
    };

    srv(GET) / "On" = [&](const request &req, buffer &data, reply &rep) {
        on = true;
        rep
            << text_plain
            << "Server is on!";
    };

    srv(GET) / "Off" = [&](const request &req, buffer &data, reply &rep) {
        on = false;
        rep
            << text_plain
            << "Server is off!";
    };

    srv(GET) / "Quit" = [&](const request &req, buffer &data, reply &rep) {
        on = false;
        rep
            << text_plain
            << "Server is quitting!";
        cv.notify();
    };

    // Bind and listen
    srv(make_endpoint_local("/tmp/nx"));

    cv.wait();
    nx::stop();
}