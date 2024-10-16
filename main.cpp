#include "../include/Config.hpp"
#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include <fstream>
#include <vector>
#include <sstream>


int main(int argc, char **argv)
{
    Server server;
    Config config;
    Client client;

    if (argc == 2)
        config.parse_config(argv[1]);
    else
        config.default();
    //get server elemtns
    Server.run();
    return 0;
}