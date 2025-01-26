#include "../include/Config.hpp"

/**
 * @brief Entry point of the server.
 *
 * Checks the command line arguments and calls checkConfig() to parse the
 * configuration file. If no arguments are provided, it defaults to
 * "configs/default.conf".
 */
int main(int argc, char **argv) {
    Config config;

    if (argc == 1)
        config.checkConfig("configs/default.conf");
    else
        config.checkConfig(argv[1]);
	
    return 0;
}


/*
CGI (Common Gateway Interface) is a standard for web servers to interact with external programs (called CGI scripts) to dynamically generate web content. 
This means that instead of simply serving static files (like HTML), the web server can run a program (written in C++, Python, Perl, etc.), 
pass the HTTP request data to it, and then use the program's output as the response for the client.

*/