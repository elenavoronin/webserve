#include "../include/Client.hpp"
#include "../include/HttpRequest.hpp"
#include <unistd.h>


Client::Client() : clientSocket(0), Http(new HttpRequest()) {}
/*
1. Shadowing Issue in Client Constructor

In your Client constructor, you are creating a local pointer Http instead of assigning to the class member Http. This is causing a memory leak and potential issues when the destructor tries to delete the uninitialized class-level Http pointer.
Fix:

In the constructor, remove the local declaration and directly initialize the class member Http.*/
Client::~Client(){
	// if (clientSocket != -1)
	// 	close(clientSocket);
	// std::cout << "~Client() 1 " << std::endl;
	if (Http != nullptr){
		delete Http;
		Http = nullptr;
	}
	// std::cout << "~Client() 2 " << std::endl;
}

Client& Client::operator=(const Client& copy){
	// std::cout << "Client::operator= 1 " << std::endl;
	if (this != &copy) {
		clientSocket = copy.clientSocket;
		if (Http != nullptr)
			delete Http;
		Http = new HttpRequest(*copy.Http);
	}
	// std::cout << "Client::operator= 2 " << std::endl;
	return *this;
}

Client::Client(const Client& copy){
	// std::cout << "Client(const Client& copy 1 " << std::endl;
	clientSocket = copy.clientSocket;
	Http = new HttpRequest(*copy.Http);
	// std::cout << "Client(const Client& copy 2 " << std::endl;
}

void Client::setSocket(int i){
	clientSocket = i;
	// std::cout << "Socket " << clientSocket << std::endl;
}

int Client::getSocket(){
	return clientSocket;
}