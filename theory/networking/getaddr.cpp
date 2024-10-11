#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h>

// int getaddrinfo(const char *node,     // e.g. "www.example.com" or IP
//                 const char *service,  // e.g. "http" or port number
//                 const struct addrinfo *hints,
//                 struct addrinfo **res); 
/*You give this function three input parameters, and it gives you a pointer 
to a linked-list, res, of results*/
int main(){
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;
	
	//I create and fill the struct hints with information I want:
	memset(&hints, 0, sizeof hints); //make sure the struct is empty

    hints.ai_family = AF_UNSPEC;      // Don't care if it's IPv4 (AF_INET) or IPv6 (AF_INET6)
    hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;      // Use my IP address automatically

	if ((status = getaddrinfo("example.com", "80", &hints, &servinfo)) != 0) {
		std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
		return 1;
	}
	std::cout << "Got the information" << std::endl;
/*
If everything works, servinfo will point to a linked list of struct addrinfos, 
each of which contains a struct sockaddr of some kind that we can use later! 
*/

//int socket(int domain, int type, int protocol); 
	int clientSocket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (clientSocket == -1) {
		std::cerr << "Socket creation failed!" << std::endl;
		freeaddrinfo(servinfo);
		return 1;
	}
	std::cout << "Socket is created" << std::endl;
	if (connect(clientSocket, servinfo->ai_addr, servinfo->ai_addrlen) == -1){
		std::cerr << "Socket connection failed!" << std::endl;
		close(clientSocket);
		freeaddrinfo(servinfo);
		return 1;		
	}
	std::cout << "Connection is successfull" << std::endl;
	freeaddrinfo(servinfo);

	const char* request = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
	/*
	GET /: This is the HTTP method (GET), and / refers to the root page of the website (the homepage).
	HTTP 1.1 is the standard
	\r\n: used in HTTP headers to mark the end of each line.
	The Host header specifies the domain name of the server
	Connection: close: tells the server that after sending the response, it should close the connection. 
	\r\n\r\n: The two carriage return/newline pairs signal the end of the headers 
		and the beginning of the body. In this case, there’s no body (since it’s a GET request), 
		but it's required to properly format the request.
	*/
		if (send(clientSocket, request, strlen(request), 0) == -1) {
		std::cerr << "Send failed!" << std::endl;
		close(clientSocket);
		return 1;
	}
	std::cout << "Request is sent" << std::endl;
	char buffer[1024];
	int bytesReceived;
	while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
		buffer[bytesReceived] = '\0';  // Null-terminate the received data
		std::cout << buffer;
	}
	if (bytesReceived == -1) {
		std::cerr << "Receive failed!" << std::endl;
	}

	// Close the socket
	close(clientSocket);
	return 0;
}