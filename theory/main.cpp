#include <iostream>
#include <string>
#include <sstream>
#include "include/TcpServer.hpp"


int main(int argc, char **argv)
{
    Server server;

    if (argc <= 2)
    {
    // if (argc == 1)
        // we need is to use a default path
    // if (argc == 2)
        //parse configuration file
    if (server.createServer(&server) == 0)
        std::cout << "Server created" << std::endl;
    else
    {
        std::cout << "Server creation failed" << std::endl;
        return 1;
    }
    if (server.createClient(&server) == 0)
        std::cout << "Client created" << std::endl;
    else
    {
        std::cout << "Client creation failed" << std::endl;
        return 1;
    }
    }
    else
    {
		std::cerr << "Error: wrong number of arguments " << std::endl;
		return 1;
	}
    }
    else
    {
		std::cerr << "Error: wrong number of arguments " << std::endl;
		return 1;
	}

    return 0;
}