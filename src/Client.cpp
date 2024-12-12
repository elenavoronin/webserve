#include "../include/Client.hpp"
#include "../include/HttpRequest.hpp"
#include <unistd.h>


Client::Client() : _clientSocket(-1), _HttpRequest(new HttpRequest()), _HttpResponse(new HttpResponse()), _CGI(NULL) {}
/*
1. Shadowing Issue in Client Constructor

In your Client constructor, you are creating a local pointer Http instead of assigning to the class member Http. This is causing a memory leak and potential issues when the destructor tries to delete the uninitialized class-level Http pointer.
Fix:

In the constructor, remove the local declaration and directly initialize the class member Http.*/
Client::~Client(){
	// if (_clientSocket != -1)
	// 	close(_clientSocket);
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

void Client::setSocket(int clientSocket){
	_clientSocket = clientSocket;
	// std::cout << "Socket " << clientSocket << std::endl;
}

int Client::getSocket(){
	return _clientSocket;
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

int Client::getCgiRead(){
	if (this->_CGI == NULL)
		return -1;
	return this->_CGI->getReadFd();
}

int Client::getCgiWrite(){
	if (this->_CGI == NULL)
		return -1;
	return this->_CGI->getWriteFd();
}

void Client::startCgi(HttpRequest *request) {
	if (this->_CGI != NULL)
		throw std::runtime_error("already initialized");
	this->_CGI = new CGI(request);
}

void Client::readFromCgi() {
	if (this->_CGI == NULL)
		throw std::runtime_error("CGI not initialized");
	//check bool if done reading
		//decide continue reading
		//or build response
		//or error?
	//signal kill if done ?
}

void Client::readFromSocket(Server *server) {
	char buf[100] = {0};
	int contentLength;

	int received = recv(_clientSocket, buf, sizeof(buf), 0);
	if (received <= 0) {
		throw std::runtime_error("no bytes to be read");
	}
	//save the request in _strReceived
	_HttpRequest->getStrReceived().append(buf, received);
	if (!_HttpRequest->isHeaderReceived()) {
		if (_HttpRequest->getStrReceived().find("\r\n\r\n") != std::string::npos) {
			contentLength = _HttpRequest->findContentLength(_HttpRequest->getStrReceived());
			if (static_cast<int>(_HttpRequest->getStrReceived().length() - _HttpRequest->getStrReceived().find("\r\n\r\n") - 4) >= contentLength)
				_HttpRequest->setHeaderReceived(true);
		}
	}
	if (_HttpRequest->isHeaderReceived()) {
		server->processClientRequest(*this, _HttpRequest->getStrReceived(), _HttpRequest);
		_HttpRequest->setHeaderReceived(false);
		_HttpRequest->clearStrReceived();
	}
	else if (received == 0) {
        // std::cout << "Client closed connection: " << client_fd << std::endl;
        // close(client_fd);
        // removeClient(pfds, i, client_fd);
    } else {
        // An error occurred with recv
		throw std::runtime_error("400"); // TODO need to link to http response
		//return proper http response
    }
}

void Client::writeToSocket(Server *server) {
	int bytesToWrite = WRITE_SIZE;
    int bytesWritten = 0;

    if (bytesToWrite > _HttpResponse->getFullResponse().size() - _responseIndex) {
        bytesToWrite = _HttpResponse->getFullResponse().size() - _responseIndex;
    }
	//data + offset inputindex 
    bytesWritten = write(_clientSocket, _HttpResponse->getFullResponse().data() + _responseIndex, bytesToWrite);
    _responseIndex += bytesWritten;
}