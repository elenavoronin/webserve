#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>

#define PORT "8080"
#define BACKLOG 10
#define MAXCLIENTS 2


void sendHttpResponse(int client_fd) {
    const char* response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, World!";
    send(client_fd, response, strlen(response), 0);
}

int setupServerSocket(){
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
		continue;			
		}
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
	if (newConnect == NULL){
		std::cerr << "Server: bind error 2??? " << std::endl;
		exit(1);
	}
	if (listen(serverSocket, BACKLOG) == -1){
		std::cerr << "Listen error " << std::endl;
		exit(1);		
	}
	std::cout << "Server: connecting ..." << std::endl;
	return serverSocket;
}


int main(){

	int clientSocket;
	int serverSocket = setupServerSocket();
	struct pollfd fds[MAXCLIENTS];
	fds[0].fd = serverSocket; //This is the server's listening socket. 
	//We wait for it to become ready for new connections.
	fds[0].events = POLLIN; //There is data to read


	int nfds = 1; // Number of file descriptors to poll
    int timeout = 30000; // Timeout for poll() in milliseconds

	for (int i = 1; i < MAXCLIENTS; ++i) {
		fds[i].fd = -1;  // -1 means unused
	}
	/*
	poll() waits for one
	poll() waits for one of a set of file descriptors to become ready to perform I/O. 
	accept() When a client connects to the server, it accepts the incoming connection request, creating a new socket (clientSocket) specifically for that client.
	recv reads data from the connected client into the buffer. It returns the number of bytes received or 0 if the connection is closed
	sendHttpResponse sends a basic HTTP response to the client using the socket descriptor fds[i].fd
	After receiving data from client the fd is closed and poll checks again for the connections
	*/
	while(true){
		if (poll(fds, nfds, timeout) == -1) //returns the amount of fds, which return events
			std::cout << "Poll error" << std::endl;
		if (fds[0].revents & POLLIN){ //The & (bitwise AND) operator compares the binary values of fds[0].revents and POLLIN. If the POLLIN bit is set in fds[0].revents, 
		//the result will be non-zero, and the condition will evaluate as true.
			struct sockaddr_storage clientsAddr;
			socklen_t clientsAddrSize = sizeof clientsAddr;
			if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientsAddr, &clientsAddrSize)) == -1){
				std::cout << "Accept error" << std::endl;
			}
			else{
				std::cout << "New connection accepted" << std::endl;
				for (int i = 1; i < MAXCLIENTS; i++){
					if (fds[i].fd == -1){
						fds[i].fd = clientSocket;
						fds[i].events = POLLIN;
						nfds = std::max(nfds, i+1);
						break;
					}
				}
			}
		}
		for(int i = 1; i < nfds; i++){
			if(fds[i].fd != -1 && (fds[i].revents & POLLIN)){
				char buffer[1024] = {0};
				int received = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
				if (received <= 0){
					std::cout << "Connection closed, received " << received << std::endl;
					close(fds[i].fd);
					fds[i].fd = -1;
				}
				else{
					std::cout << "Connection closed, received " << buffer << std::endl;
					sendHttpResponse(fds[i].fd);
					close(fds[i].fd);
					fds[i].fd = -1;
				}
			}
		}
	}
	close(serverSocket);
	return 0;

}

/*
compile webserver, in other windows connect with "curl http://localhost:8080"
write thread simulation with several clients!
*/

Generate ssh keys 
key gen?
