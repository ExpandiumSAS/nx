#include <iostream>
#include <nx/nx.hpp>

#include <cstdlib>
#include <cstring>
#include <vector>

using namespace nx;

int main(int ac, char **av)
{
    httpc cli;
    std::string user_input;
    while (std::cin >> user_input)
    {
        cli(
            GET,
            make_endpoint_local("/tmp/nx")) /
            user_input = [&](const reply &rep, buffer &data) {
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