#include "Client.hpp"
#include "../Server.hpp"
#include <iostream>     // for std::cout
#include <vector>      // for std::vector
#include <poll.h>      // for poll() and struct pollfd
#include <sys/types.h> // for basic types
#include <sys/socket.h> // for socket-related functions and structures
#include <netinet/in.h> // for sockaddr_in, sockaddr_storage

void Server::run() {
	std::vector<struct pollfd> pfds;
	std::cout << "check";
	int listener = report_ready(pfds);
	
	while (true) {
		int poll_test = poll(pfds.data(), pfds.size(), -1);
		if (poll_test == -1) {
			std::cout << "Poll error" << std::endl;
			return;
		}
		
		for (size_t i = 0; i < pfds.size(); i++) {
			if (pfds[i].revents & POLLIN) {
				if (pfds[i].fd == listener) {
					handle_new_connection(listener, pfds);
				} else {
					handle_client_data(pfds, i, listener);
				}
			}
		}
	}
}


int main() {
    Server server;
    server.run();
    return 0;
}