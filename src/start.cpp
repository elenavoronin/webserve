#include "../include/Client.hpp"
#include "../include/Config.hpp"
#include "../include/Server.hpp"
#include <iostream>     // for std::cout
#include <vector>      // for std::vector
#include <poll.h>      // for poll() and struct pollfd
#include <sys/types.h> // for basic types
#include <sys/socket.h> // for socket-related functions and structures
#include <netinet/in.h> // for sockaddr_in, sockaddr_storage

int main(int argc, char **argv) {
	//for creating server here I need to get info from config: port, server_name.....
    Config config;

    if (argc == 1)
        config.check_config("configs/default.conf");
    else
        config.check_config(argv[1]);
	
    return 0;
}

/*
to test non blocking:

# In terminal 1
curl -X POST --data-binary "@large_file_1.txt" http://localhost:8080/endpoint

# In terminal 2
curl -X POST --data-binary "@large_file_2.txt" http://localhost:8080/endpoint

# In terminal 3
curl -X POST --data-binary "@large_file_3.txt" http://localhost:8080/endpoint

*/

/*
TODO check if the methods are allowed in config file, otherwise only GET by default
TODO parse root, index and so on from config in handle request
TODO do we accept HTTP/1.0?
TODO do we need broadcast message?
done write getters for config
TODO figure out locations
TODO solve memory allocation issues
TODO Rename methods with CamelCase
TODO  buffer << file.rdbuf(); //read file by bytes, go back to poll, check if finished reading
TODO add charset 8 for Content type



*/
