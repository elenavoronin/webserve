#include "../include/Client.hpp"
#include "../include/HttpRequest.hpp"
#include <unistd.h>


Client::Client() : _clientSocket(0), _HttpRequest(new HttpRequest()), _HttpResponse(new HttpResponse()), _i(0) {}
/*
1. Shadowing Issue in Client Constructor

In your Client constructor, you are creating a local pointer Http instead of assigning to the class member Http. This is causing a memory leak and potential issues when the destructor tries to delete the uninitialized class-level Http pointer.
Fix:

In the constructor, remove the local declaration and directly initialize the class member Http.*/
Client::~Client(){
	// if (clientSocket != -1)
	// 	close(clientSocket);
	// std::cout << "~Client() 1 " << std::endl;
	// if (_HttpRequest != nullptr){
	// 	delete _HttpRequest;
	// 	_HttpRequest = nullptr;
	// }
	// 	if (_HttpResponse != nullptr){
	// 	delete _HttpResponse;
	// 	_HttpResponse = nullptr;
	// }
	// std::cout << "~Client() 2 " << std::endl;
}

Client& Client::operator=(const Client& copy){
	// std::cout << "Client::operator= 1 " << std::endl;
	if (this != &copy) {
		_clientSocket = copy._clientSocket;
		if (_HttpRequest != nullptr)
			delete _HttpRequest;
		_HttpRequest = new HttpRequest(*copy._HttpRequest);
		if (_HttpResponse != nullptr)
			delete _HttpResponse;
		_HttpResponse = new HttpResponse(*copy._HttpResponse);
	}
	// std::cout << "Client::operator= 2 " << std::endl;
	return *this;
}

Client::Client(const Client& copy){
	// std::cout << "Client(const Client& copy 1 " << std::endl;
	_clientSocket = copy._clientSocket;
	_HttpRequest = new HttpRequest(*copy._HttpRequest);
	_HttpResponse = new HttpResponse(*copy._HttpResponse);
	// std::cout << "Client(const Client& copy 2 " << std::endl;
}

void Client::setSocket(int i){
	_clientSocket = i;
	// std::cout << "Socket " << clientSocket << std::endl;
}

int Client::getSocket(){
	return _clientSocket;
}

void Client::setI(int i){
	_i = i;
}

int Client::getI(){
	return _i;
}

void Client::setHttpRequest(HttpRequest* httpRequest){
	_HttpRequest = httpRequest;
}

void Client::setHttpResponse(HttpResponse* httpResponse){
	_HttpResponse = httpResponse;
}

HttpRequest* Client::getHttpRequest() const{
	return _HttpRequest;
}

HttpResponse* Client::getHttpResponse() const{
	return _HttpResponse;
}
