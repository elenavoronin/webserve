
#include <sys/socket.h>
#include <netinet/in.h>


#include <iostream>
#include <unistd.h>
#include <cstring>


const int PORT = 8080;

int main()
{
	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8080);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	const char* message = "PRIVET";
	send(clientSocket, message, strlen(message), 0);
	/*
	clientSocket is the socket descriptor you want to send data to
	send() returns the number of bytes actually sent out—
	this might be less than the number you told it to send! 
	See, sometimes you tell it to send a whole gob of data and 
	it just can’t handle it. It’ll fire off as much of the data as 
	it can, and trust you to send the rest later. Remember, 
	if the value returned by send() doesn’t match the value in len, 
	it’s up to you to send the rest of the string. The good news is this: if the packet is small (less than 1K or so) 
	it will probably manage to send the whole thing all in one go. 
	*/

}