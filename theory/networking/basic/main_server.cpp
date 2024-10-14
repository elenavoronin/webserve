
#include <sys/socket.h>
//what for
#include <netinet/in.h>
#include <functional> //std::bind
#include <iostream>
#include <unistd.h>
#define BACKLOG 10  // how many pending connections queue will hold

const int PORT = 8080;

int main() //add argc, argv
{
	//create socket
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	/*socketfd: It is the file descriptor for the socket.
	AF_INET: It specifies the IPv4 protocol family.
	SOCK_STREAM: It defines that the TCP type socket.*/

	//define Server Address
	sockaddr_in serverAddress; // It is the data type that is used to store the address of the socket.
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT); //This function is used to convert the unsigned int from machine byte order to network byte order.
	serverAddress.sin_addr.s_addr = INADDR_ANY; //It is used when we don’t want to bind our socket to any particular IP and instead make it listen to all the available IPs.

	//std::bind - “binding” a function(serverSocket) to a specific object or value (struct sockaddr*)&serverAddress), 
	//allowing it to be called with a different number of arguments (sizeof(serverAddress) or with a different order of arguments.
	bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	listen(serverSocket, BACKLOG); //BACKLOG (10) is the number of clients that are allowed to be queded up in a "connection line" to the server. 
	//Client:
	int clientSocket = accept(serverSocket, nullptr, nullptr); //why NULL?
	/*
	accept creates a new connected socket
	addr is a pointer to a sockaddr structure
	The addrlen argument is a value-result argument: the caller must initialize it to contain the size (in bytes) of the structure pointed to by addr
		|	If the socket is marked
		|	nonblocking and no pending connections are present on the queue,
		|	accept() fails with the error EAGAIN or EWOULDBLOCK.
	
		In order to be notified of incoming connections on a socket, you
		can use select(2), poll(2), or epoll(7).  A readable event will
		be delivered when a new connection is attempted and you may then
		call accept() to get a socket for that connection.	
	*/
	char buffer[1024] = {0};
	recv(clientSocket, buffer, sizeof(buffer), 0);
	/*
	recv() returns the number of bytes actually read into the buffer
	recv() can return 0. This can mean only one thing: the remote side has closed the connection on you! 
	0 - flag*/
	std::cout << "I'm in server, receive message from client: " << buffer << std::endl;
	close(serverSocket);
}
