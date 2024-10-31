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


/*Function to run the server and handle incoming connections and data
- pfds: Vector to hold poll file descriptors
- listener: File descriptor for the server listening socket
- poll_test: Result of poll() function to check file descriptors*/
void Server::run() {
	std::vector<struct pollfd> pfds;
	std::cout << "check in Server::run" << std::endl;
	int listener = report_ready(pfds);
	int poll_test;
	

	while (true) {
		poll_test = poll(pfds.data(), pfds.size(), -1);
		if (poll_test == -1) {
			std::cout << "Poll error" << std::endl;
			return;
		}
		
		for (size_t i = 0; i < pfds.size(); i++) {
			if (pfds[i].revents & POLLIN) {
				if (pfds[i].fd == listener) {
					handle_new_connection(listener, pfds, i);
				} else {
					handle_client_data(pfds, i, listener);
				}
			}
		}
	}
}

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
void Server::addClient(std::vector<struct pollfd> &pfds, int clientSocket, int i){
	Client newClient;
	newClient.i = i;
	newClient.setSocket(clientSocket);
	clients.push_back(newClient);
	add_to_pfds(pfds, clientSocket);
	// std::cout << "Add client" << std::endl;
}

/*Function to remove a client connection from the server
- pfds: Vector of pollfd structures
- i: Index of the pollfd to remove
- clientSocket: The client socket that needs to be closed*/
void Server::removeClient(std::vector<struct pollfd> &pfds, int i, int clientSocket){
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
void Server::handle_new_connection(int listener, std::vector<struct pollfd> &pfds, int i){
	struct sockaddr_storage clientsAddr;
    socklen_t clientsAddrSize = sizeof(clientsAddr);
    int newfd = accept(listener, (struct sockaddr *)&clientsAddr, &clientsAddrSize);
    if (newfd == -1) {
        perror("accept");
    } else 
		addClient(pfds, newfd, i);
}

/*Function to broadcast a message to all clients except the sender
- sender_fd: File descriptor of the client sending the message
- buf: Buffer containing the message to broadcast
- received: Number of bytes received in the message
- pfds: Vector of pollfd structures to send the message to
- listener: The listener file descriptor (to avoid sending to it)*/
// void Server::broadcast_message(int sender_fd, char *buf, int received, std::vector<struct pollfd> &pfds, int listener){
//     for (size_t j = 0; j < pfds.size(); j++) {
//         int dest_fd = pfds[j].fd;
//         if (dest_fd != listener && dest_fd != sender_fd) {
//             if (sendall(dest_fd, buf, &received) == -1) {
//                 std::cout << "Can't send" << std::endl;
//             }
//         }
//     }
// }


// Function to send all data in the buffer to the specified socket
// - s: The socket to send data to
// - buf: The buffer containing the data to send
// - len: Pointer to the length of data to send (will be updated with the amount actually sent)
// int Server::sendall(int s, char *buf, int *len){
// 	int total = 0;
// 	int bytesleft = *len;
// 	int n;

// 	while(total < *len){
// 		n = send(s, buf + total, bytesleft, 0);
// 		if (n == -1)
// 			break;
// 		total += n;
// 		bytesleft -= n;
// 	}
// 	*len = total;
// 	return (n == -1) ? -1 : 0; 
// }



// Function to handle data received from a connected client
// - pfds: Vector of pollfd structures
// - i: Index of the pollfd that has client data ready
// - listener: File descriptor for the listener socket (used to avoid sending data back to it)
void Server::handle_client_data(std::vector<struct pollfd> &pfds, int i, int listener){
	int contentLength = 0;
	char buf[20] = {0};
	int sender_fd = pfds[i].fd;
	Client* client;
	for (auto& c : clients){ //find the Client
		if (c.getSocket() == sender_fd)
			client = &c;
	}
	int received = recv(sender_fd, buf, sizeof(buf), 0);
	if (received > 0){ //The only difference between recv() and read(2) is the presence of flags. 
		client->Http->_strReceived.append(buf, received); //save the request in _strReceived
		if (client->Http->_strReceived.find("\r\n\r\n") != std::string::npos && !client->Http->headerReceived){
			client->Http->headerReceived = true;
			contentLength = client->Http->findContentLength(client->Http->_strReceived);
			processClientRequest(sender_fd, client->Http->_strReceived,client->Http);
				// broadcast_message(sender_fd, buf, received, pfds, listener);
			client->Http->_strReceived.clear();
			return;
		}
		if (client->Http->headerReceived && client->Http->_strReceived.length() >= contentLength) //fix it
			return ;
	}
	if (received <= 0) {
		if (received == 0)
			std::cout << "pollserver: socket" << sender_fd << "hung up\n";
		close(sender_fd);
		removeClient(pfds, i, sender_fd);
	}
}

