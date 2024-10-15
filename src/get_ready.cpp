#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <iostream>
#include <vector>

#define BACKLOG 10

int Server::get_listener_socket(){
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;
	struct addrinfo *newConnect;

	int serverSocket;
	int opt = 1;

	memset(&hints, 0, sizeof hints); //is used to clear out the hints struct. Then, we fill in some details:
	hints.ai_family = AF_UNSPEC; // allows either IPv4 or IPv6.
	hints.ai_socktype = SOCK_STREAM; // tells the system to use TCP
	hints.ai_flags = AI_PASSIVE; //makes the program automatically fill in the IP 
	if ((status = getaddrinfo(NULL, this->port, &hints, &servinfo)) != 0){
		std::cout << "Error get Address information" << std::endl;
		return 1;
	}
	for (newConnect = servinfo; newConnect != NULL; newConnect= newConnect->ai_next){
		if ((serverSocket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1){ //creates a socket
			std::cout << "Create server socket " << serverSocket << std::endl;
			continue;
		}
		setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //allows the program to reuse the address
		if (bind(serverSocket, newConnect->ai_addr, newConnect->ai_addrlen) == -1){ //associates the socket with an address (IP and port).
			std::cout << "Bind error" << std::endl;
			close(serverSocket);
			continue;
		}
		break;
	}
	freeaddrinfo(servinfo);
	if (newConnect == NULL) //If no address was successfully bound, the program exits with an error.
		exit(1);
	if (listen(serverSocket, BACKLOG) == -1) //tells the socket to listen for incoming connections
		return -1;
	std::cout << "serverSocket " << serverSocket << std::endl;
	return serverSocket;
}

void Server::add_to_pfds(std::vector<struct pollfd> &pfds, int newfd){
	struct pollfd pfd;
    pfd.fd = newfd;
    pfd.events = POLLIN;
    pfds.push_back(pfd);
}

void Server::del_from_pfds(std::vector<struct pollfd> &pfds, int i){
	pfds[i] = pfds.back(); // Move the last one to the deleted spot
	pfds.pop_back(); // Remove the last one (which is now duplicated)
}

int Server::report_ready(std::vector<struct pollfd> &pfds){
	int listener = get_listener_socket(); // Set up and get a listening socket
	if (listener == -1){
		std::cout << "Error get listener socket" << std::endl;
		exit(1);
	}
	struct pollfd pfd; //add a new file descriptor to the vector of pollfd
	pfd.fd = listener;
	pfd.events = POLLIN; // Report ready to read on incoming connection
	pfds.push_back(pfd);
	return listener;
}