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

/*Function to add a new file descriptor to the poll vector
- pfds: Vector of pollfd structures
- newfd: The file descriptor of the new connection to be added*/
void Server::add_to_pfds(std::vector<struct pollfd> &pfds, int newfd){
	struct pollfd pfd;
    pfd.fd = newfd;
    pfd.events = POLLIN;
    pfds.push_back(pfd);
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

/*Function to add a client connection to the server
- pfds: Vector of pollfd structures
- clientSocket: The file descriptor for the client socket to add*/
void Server::addClient(std::vector<struct pollfd> &pfds, int clientSocket){
	Client newClient;
	newClient.setSocket(clientSocket);
	clients.push_back(newClient);
	add_to_pfds(pfds, clientSocket);
	//std::cout << "Add client" << getPortStr() << " I " << i << " clientSocket " << clientSocket << std::endl;
}

/*Function to remove a client connection from the server
- pfds: Vector of pollfd structures
- i: Index of the pollfd to remove
- clientSocket: The client socket that needs to be closed*/
void Server::removeClient(std::vector<struct pollfd> &pfds, int i, int clientSocket){
	
	(void)clientSocket;//unvoid
	
	del_from_pfds(pfds, i);
	// for (auto it = clients.begin(); it != clients.end(); ++it) {
	// 	if (it->getSocket() == clientSocket) {
	// 		clients.erase(it);
	// 		break;
	// 	}
	// }
	// close(clientSocket);
}

/*Function to handle new incoming connections
- listener: The file descriptor for the listener socket
- pfds: Vector of pollfd structures where the new connection will be added*/
void Server::handle_new_connection(std::vector<struct pollfd> &pfds){
	struct sockaddr_storage clientsAddr;
    socklen_t clientsAddrSize = sizeof(clientsAddr);
    int newfd = accept(listener_fd, (struct sockaddr *)&clientsAddr, &clientsAddrSize);
	if (!newfd){
		std::cerr << "Can't accept a new connection" << std::endl;
		return;
	}
	std::cout << "New connection accepted: " << newfd << std::endl;
	addClient(pfds, newfd);
}

// Function to handle data received from a connected client
// - pfds: Vector of pollfd structures
// - i: Index of the pollfd that has client data ready
// - listener: File descriptor for the listener socket (used to avoid sending data back to it)
void Server::handle_client_data(std::vector<struct pollfd> &pfds, int i){
/*Add POLLOUT when I reached the length, don't close cause I still need to connect to client*/
	// unsigned long contentLength = 0;//changed from int to unsigned long because of flags
	char buf[100] = {0};
	int client_fd = pfds[i].fd;
	Client* client = nullptr;
	int contentLength;
	for (auto& c : clients){ //find the Client
		if (c.getSocket() == client_fd){ //TODO add file fd and/or pipe fd to client to read in chunks
			client = &c;
			break;
		}
	}
	if (!client) {
		// std::cerr << "Client not found!" << std::endl; //TODO remove later
		return;
	}
	int received = recv(client_fd, buf, sizeof(buf), 0);
	if (received > 0){ //The only difference between recv() and read(2) is the presence of flags. 
		HttpRequest* request = client->getHttpRequest();
		request->getStrReceived().append(buf, received); //save the request in _strReceived
			if (!request->isHeaderReceived()) {
				if (request->getStrReceived().find("\r\n\r\n") != std::string::npos) {
					contentLength = request->findContentLength(request->getStrReceived());
					// processClientRequest(client_fd, request->getStrReceived(), request);
					// size_t bodyStart = request->getStrReceived().find("\r\n\r\n") + 4;
					// std::cout << &request << " content length " << contentLength  << "bodystart" << bodyStart << " last thing" <<  request->getStrReceived().find("\r\n\r\n") + 4 << std::endl;
					 std::cout << "hi "<<request->getStrReceived().length()<< " " << contentLength << std::endl;
					// request->_bodyReceived = request->getStrReceived().substr(bodyStart);
					if (static_cast<int>(request->getStrReceived().length() - request->getStrReceived().find("\r\n\r\n") - 4) >= contentLength)
						request->setHeaderReceived(true);
				}
			}
			if (request->isHeaderReceived()) { //reading the file
					// std::cout << "first" << std::endl;
					processClientRequest(client_fd, request->getStrReceived(), request);
					// std::cout << "second" << std::endl;
					// request->_readyToSendBack = true;
					// std::cout << "third" << std::endl;
					request->setHeaderReceived(false);
					request->clearStrReceived();
					//close(client_fd);
					//removeClient(pfds, i, client_fd);
					//pfds[i].events |= POLLOUT;  //TODO add it right after finishing reading, before sending respond
			}
			// if (request->_readyToSendBack == true){
			// 	//sendRespond();
			// }
	}
	else if (received == 0) {
        // std::cout << "Client closed connection: " << client_fd << std::endl;
        // close(client_fd);
        // removeClient(pfds, i, client_fd);
    } else {
        // An error occurred with recv
        perror("ppp");
        close(client_fd);
        removeClient(pfds, i, client_fd);
    }
}

void Server::sendResponse(int clientSocket, const std::string& response) {
	 write(clientSocket, response.c_str(), response.size());
}



void Server::checkLocations(std::string path) {
    if (path == this->getIndex()) {
		return;
	}//TODO figure out when to reset server information to default
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