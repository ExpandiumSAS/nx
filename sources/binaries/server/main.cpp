#include <iostream>

#include <nx/nx.hpp>

#include <cstdlib>
#include <cstring>
#include <vector>

using namespace nx;

int main(int ac, char **av)
{
    bool on = true;
    httpd srv;
    httpc cli;
    cond_var cv;
    // Register a GET handler
    srv(GET) / "Hello" = [&](const request &req, buffer &data, reply &rep) {
        if (on)
        {
            const auto& target = req.h("target");
            rep.postpone();
            cli(
                GET,
                make_endpoint("/tmp/nx_" + target)) /
                "HELLO_NAME" = [&](const reply &call_rep, buffer &call_data) {
                    rep
                        << text_plain
                        << call_data;
                    rep.done();
                };
        }
    };

    srv(GET) / "Bye" = [&](const request &req, buffer &data, reply &rep) {
        if (on)
        {
            const auto& target = req.h("target");
            rep.postpone();
            cli(
                GET,
                make_endpoint("/tmp/nx_" + target)) /
                "BYE_NAME" = [&](const reply &call_rep, buffer &call_data) {
                    rep
                        << text_plain
                        << call_data;
                    rep.done();
                };
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
        cv.notify();
    };

    // Bind and listen
    srv(make_endpoint("127.0.0.1", 4242));

    cv.wait();
    nx::stop();
}