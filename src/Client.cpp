#include "../include/Client.hpp"
#include <unistd.h>

Client::Client(){
	clientSocket = 0;
}

Client::~Client(){
	if (clientSocket != -1)
		close(clientSocket);
	delete this;
}

Client& Client::operator=(const Client& copy){
	if (this != &copy)
		clientSocket = copy.clientSocket;
	return (*this);
}

Client::Client(const Client& copy){
	clientSocket = copy.clientSocket;
}

void Client::setSocket(int i){
	clientSocket = i;
	std::cout << "Socket " << clientSocket << std::endl;
}

int Client::getSocket(){
	return clientSocket;
}