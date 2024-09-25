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

#define PORT "9034"
#define BACKLOG 10

void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size){
	if (*fd_count == *fd_size){
		*fd_size *=2;
		*pfds = (struct pollfd *)realloc(*pfds, sizeof(**pfds) * (*fd_size));
	}
	(*pfds)[*fd_count].fd = newfd;
	(*pfds)[*fd_count].events = POLLIN;
	(*fd_count)++;
}

void del_from_pfds(struct pollfd pfds[],int i, int *fd_count){
	pfds[i] = pfds[*fd_count - 1];
	(*fd_count)--;
}

int get_listener_socket(){
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
	if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0){
		//looks up network addresses based on the criteria specified in the hints. 
		//The goal is to find valid IP addresses and port combinations that the server can use.
		// servinfo list contains all the network interfaces (IP addresses) and port options that are available on the server. 
		// The server can then choose one of these addresses to set up a listening socket.
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

/*
We create a listener socket, that will control new connections
In the while loop:
	The server listens for new connections.
	When a new client connects, it is accepted, and the client connection is added to be monitored.
OR
	The server reads and processes data from existing client connections.
	If a client sends data, it forwards this data to other connected clients.
	If a client disconnects, its socket is removed from the pollfd array.
*/

int main(){
	struct sockaddr_storage clientsAddr;
	socklen_t clientsAddrSize;
	char buf[256];
	int newfd;
	int fd_count = 1;

	int fd_size = 5; //Start off with room for 5 connections
	struct pollfd *pfds = (struct pollfd *)malloc(sizeof *pfds * fd_size);
	if (pfds <= 0)
		std::cout << "Allocation error" << std::endl;

	int listener = get_listener_socket(); // Set up and get a listening socket
	if (listener == -1){
		std::cout << "Error get listener socket" << std::endl;
		exit(1);
	}
	pfds[0].fd = listener;
	pfds[0].events = POLLIN; // Report ready to read on incoming connection
	while(true){
		int poll_test = poll(pfds, fd_count, -1);
		if (poll_test == -1){ //returns the amount of fds, which return events
			std::cout << "Poll error" << std::endl; //exit?
		}
		for(int i = 0; i < fd_count; i++){ // Run through the existing connections looking for data to read
			if (pfds[i].revents & POLLIN){ //We have someone who is ready to read
				if(pfds[i].fd == listener){
/*
The listening socket is the special socket that accepts new incoming connections from clients. 
When the listener socket is ready to read (POLLIN event), 
it means that a new client is trying to connect to the server.
*/
					clientsAddrSize = sizeof clientsAddr;
					newfd = accept(listener, (struct sockaddr *)&clientsAddr, &clientsAddrSize);
					if (newfd == -1)
						perror("accept");
					else {
						add_to_pfds(&pfds, newfd, &fd_count, &fd_size);
						std::cout << "new connection. Newfd: " <<newfd << std::endl;
					}
				}
				else {
/*
The else block handles existing client connections. 
This part runs for sockets that are not the listener, 
meaning they are regular client sockets that have already been accepted.
*/
					int received = recv(pfds[i].fd, buf, sizeof buf, 0);
					std::cout << "Received bytes: " << received << std:: endl;
					int sender_fd = pfds[i].fd;
					if (received <= 0) {
						if (received == 0) {
							std::cout << "pollserver: socket" << sender_fd << "hung up\n";
						} else {
							perror("recv");
						}
						close(pfds[i].fd);
						del_from_pfds(pfds, i, &fd_count);
                    }
					else {
						for(int j = 0; j < fd_count; j++){
							int dest_fd = pfds[j].fd;
							if (dest_fd != listener && dest_fd != sender_fd){
								if (send(dest_fd, buf, received, 0) == -1) {
									std::cout << "Can't send" << std::endl;
								}
							}
						}
					}

				}
			}
		}
	}
	return 0;
}


/*
Check if the port is occupied:
lsof -i :9034

You might need to kill the PID

*/

//testing
//telnet localhost PORT
//or
//curl localhost PORT