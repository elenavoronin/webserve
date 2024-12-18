#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include "../include/HttpRequest.hpp"
#include <iostream>     // for std::cout
#include <vector>      // for std::vector
#include <poll.h>      // for poll() and struct pollfd
#include <sys/types.h> // for basic types
#include <sys/socket.h> // for socket-related functions and structures
#include <netinet/in.h> // for sockaddr_in, sockaddr_storage
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BACKLOG 10

Server::Server(){
}

Server::~Server(){}

// Server::Server(const Server& copy) {
//     _server_name = copy._server_name;
//     _port = copy._port;
//     _root = copy._root;
//     _autoindex = copy._autoindex;
//     _upload_store = copy._upload_store;
//     _allowed_methods = copy._allowed_methods;
//     _default_file = copy._default_file;
// 	_locations = copy._locations;
// }


// Server& Server::operator=(const Server& copy) {
// 	if (this != &copy) {
// 		_server_name = copy._server_name;
// 		_port = copy._port;
// 		_root = copy._root;
// 		_autoindex = copy._autoindex;
// 		_upload_store = copy._upload_store;
// 		_allowed_methods = copy._allowed_methods;
// 		_default_file = copy._default_file;
// 		_locations = copy._locations;
// 	}
// 	return *this;
// }

/*Function to create and return a listener socket
- status: Result of getaddrinfo(), checks if address info is valid
- hints: addrinfo structure with settings for creating socket
- servinfo: Linked list of address structures
- newConnect: Pointer to iterate through servinfo to create a socket
- serverSocket: File descriptor for server socket
- opt: Option value for setsockopt() to allow address reuse*/
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
	if ((status = getaddrinfo(NULL, getPortStr().c_str(), &hints, &servinfo)) != 0){
		std::cout << "Error get Address information" << std::endl;
		return 1;
	}
	for (newConnect = servinfo; newConnect != NULL; newConnect= newConnect->ai_next){
		if ((serverSocket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1){ //creates a socket
			// std::cout << "Create server socket " << serverSocket << std::endl;
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
	// std::cout << "serverSocket " << serverSocket << std::endl;
	return serverSocket;
}

/*Function to remove a file descriptor from the poll vector
- pfds: Vector of pollfd structures
- i: Index of the file descriptor to remove from the poll vector*/
void Server::del_from_pfds(std::vector<struct pollfd> &pfds, int i){
	pfds[i] = pfds.back(); // Move the last one to the deleted spot
	pfds.pop_back(); // Remove the last one (which is now duplicated)
}

/*Function to report readiness of the server and set up the listener socket
- pfds: Vector of pollfd structures where listener will be added
- listener: The file descriptor for the listener socket*/
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

/**
 * @brief Handle a new connection and add it to the list of clients and the
 * EventPoll
 *
 * This function accepts a new connection and adds it to the clients vector.
 * It also adds the new client's file descriptor to the EventPoll so that it
 * can be monitored for incoming data.
 *
 * @param eventPoll The EventPoll to add the new client to
 */
void Server::handleNewConnection(EventPoll &eventPoll){
    int new_fd = accept(listener_fd, nullptr, nullptr);
    if (new_fd == -1) {
        std::cerr << "Error accepting new connection!" << std::endl;
        return;
    }

    // Add the new client directly to the clients vector
    clients.emplace_back(new_fd);

    // Add the new client file descriptor to the EventPoll
    eventPoll.addPollFdEventQueue(new_fd, POLLIN);
}

/**
 * @brief Handle events on a file descriptor
 *
 * This function takes an index into the EventPoll's list of pollfds, and
 * handles the event(s) associated with that file descriptor. It first
 * finds the Client associated with the file descriptor, and then
 * processes the event(s) according to the type of event.
 *
 * @param eventPoll The EventPoll object containing the pollfds
 * @param i The index into the pollfds vector
 * @todo  implement client->closeConnection(); in client
 */
void Server::handlePollEvent(EventPoll &eventPoll, int i) {
    Client *client = nullptr;
    pollfd &currentPollFd = eventPoll.getPollEventFd()[i];
    int event_fd = currentPollFd.fd;

    // Find the client associated with the file descriptor
    for (auto &c : clients) {
        if (c.getSocket() == event_fd || c.getCgiRead() == event_fd || c.getCgiWrite() == event_fd) {
            client = &c;
            break;
        }
    }

    if (!client) {
        // Client not found; skip processing
        return;
    }

    // Handle readable events
    if (currentPollFd.revents & POLLIN) {
        if (event_fd != client->getSocket()) {
            client->readFromCgi();
        } else {
            client->readFromSocket(this);
        }
    }

    // Handle writable events
    if (currentPollFd.revents & POLLOUT) {
        if (event_fd != client->getSocket()) {
            // client->writeToCgi();
        } else {
            client->writeToSocket();
        }
    }

    // Handle hangup or disconnection events
    if (currentPollFd.revents & (POLLHUP | POLLRDHUP)) {
        client->closeConnection(eventPoll);

        // Remove client from the list
        clients.erase(std::remove_if(clients.begin(), clients.end(), [&](const Client &c) {
            return c.getSocket() == event_fd;
        }), clients.end());
    }
}

/**
 * @todo figure out when to reset server information to default
 */
void Server::checkLocations(std::string path) {
    if (path == this->getIndex()) {
		return;
	}
	for (const auto& location : this->getLocations()) {
		if (path == location.first) {
			if (!location.second.empty()) {
				Location loc = location.second[0];
				if (!loc.getRoot().empty())
					this->set_root(loc.getRoot());
			}
		}
	}
}