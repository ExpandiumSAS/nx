#include <iostream>
#include <nx/nx.hpp>

#include <cstdlib>
#include <cstring>
#include <vector>

using namespace nx;

int main(int ac, char **av)
{
    httpc cli;
    std::string action;
    std::string target;
    while (std::cin >> action >> target)
    {
        cli(
            GET,
            make_endpoint("127.0.0.1", 4242)) /
            action << header{ "target", target }
            = [&](const reply& rep, buffer& data) {
            if (rep)
            {
                std::cout
                    << "server replied: "
                    << data
                    << std::endl;
            }
        };
    }
}
