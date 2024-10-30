#include "../include/Client.hpp"
#include "../include/Server.hpp"
#include <iostream>     // for std::cout
#include <vector>      // for std::vector
#include <poll.h>      // for poll() and struct pollfd
#include <sys/types.h> // for basic types
#include <sys/socket.h> // for socket-related functions and structures
#include <netinet/in.h> // for sockaddr_in, sockaddr_storage

// int main() {
// 	//for creating server here I need to get info from config: port, server_name.....
//     Server server;
//     server.run();
//     return 0;
// }

/*
to test non blocking:

# In terminal 1
curl -X POST --data-binary "@large_file_1.txt" http://localhost:8080/endpoint

# In terminal 2
curl -X POST --data-binary "@large_file_2.txt" http://localhost:8080/endpoint

# In terminal 3
curl -X POST --data-binary "@large_file_3.txt" http://localhost:8080/endpoint

*/