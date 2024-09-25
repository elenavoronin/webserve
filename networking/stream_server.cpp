#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>

#define PORT "3490" //change to 80?
#define BACKLOG 10

/*
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
*/

int main(){
	int status;
	int serverSocket;
	int opt;
	struct addrinfo hints;
	struct addrinfo *servinfo;
	struct addrinfo *newConnect;

	struct sockaddr_storage clientsAddr;
	socklen_t clientsAddrSize;
	int clientSocket;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0){
		std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
		return 1;
	}
	for (newConnect = servinfo; newConnect != NULL; newConnect = newConnect->ai_next){
		if ((serverSocket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1){
		std::cerr << "Socket create error " << std::endl;
		return 1;			
		}
/*
int fcntl(int socket, int cmd, ...);
Setting a socket to non-blocking means that the operations performed 
on that socket, such as sending or receiving data, 
will not wait for the operation to complete if it can't be done immediately. 
In other words, the function calls (like recv(), send(), or accept()) will return immediately, 
even if the operation hasn't been completed yet.
fcntl(serverSocket, F_SETFL, O_NONBLOCK);
Bad idea to use it because:
fcntl modifies the state of the file descriptor, which is shared across threads.
using fcntl means you may need to handle different code paths for setting non-blocking behavior on different platforms
*/
/*
**setsockopt** allows to configure various options for sockets — like setting a timeout, 
enabling or disabling certain features, or adjusting buffer sizes.
serverSocket The socket file descriptor. 
SOL_SOCKET Specifies the protocol level at which the option exists (e.g., SOL_SOCKET for socket-level options, IPPROTO_TCP for TCP-level options).
SO_REUSEADDR This is the specific option that allows the port to be reused.
&opt Setting it to 1 means "yes" or "true," so we're saying "enable SO_REUSEADDR option."
Other Common setsockopt Options
    SO_RCVTIMEO: Set a receive timeout for the socket.
    SO_SNDTIMEO: Set a send timeout for the socket.
    SO_KEEPALIVE: Enable keepalive messages for the socket to detect dead connections.
    TCP_NODELAY: Disable Nagle's algorithm, which can reduce latency in some cases.
*/
		if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
			std::cerr << "Setsockopt error " << std::endl;
			return 1;			
		}
		if (bind(serverSocket, newConnect->ai_addr, newConnect->ai_addrlen) == -1){
			close(serverSocket);
			std::cerr << "Server: bind error " << std::endl;
			continue;			
		}
		break;
	}
	freeaddrinfo(servinfo);
	if (newConnect == NULL){  //WHY?????????????????????????????????
		std::cerr << "Server: bind error 2??? " << std::endl;
		exit(1);
	}
	if (listen(serverSocket, BACKLOG) == -1){
		std::cerr << "Listen error " << std::endl;
		exit(1);		
	}
/*
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
*/
	std::cout << "Server: connecting ..." << std::endl;
	while(1){
/*
clientsAddr is used to store address information about the client that connects to your server. 
When a client connects, the server needs to know some details about the client, 
such as their IP address and port number.
*/
		clientsAddrSize = sizeof clientsAddr;
		clientSocket = accept(serverSocket, (struct sockaddr *)&clientsAddr, &clientsAddrSize);
/*
accept() waits for an incoming connection from a client. When a client tries to connect to the server:
serverSocket is the main socket that the server is listening on for new connections.
clientSocket is a new file descriptor that is created for this specific client connection.
clientsAddr is where the address information of the client (like their IP address and port) will be stored.
clientsAddrSize tells accept() how big the their_addr structure is.
*/

		if (clientsAddrSize == 0){
			std::cerr << "Accept error " << std::endl;
			exit(1);
		}
/*
After a client connects, we want to convert their address into a human-readable form 
(so we can print the client's IP address in a format like 192.168.1.1).
inet_ntop is not allowed by subject, we can count it manually.
*/
/*
		NOT Allowed solution:
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s
*/	
	if (!fork()){ //creating a child process
		close(serverSocket);
		if (send(clientSocket, "Hello", 6, 0) == -1)
			std::cerr << "Can't send the message" << std::endl;
		close(clientSocket);
		exit(0);
	}
	close(clientSocket);
	}
	return 0;
}