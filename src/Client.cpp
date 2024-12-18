#include "../include/Client.hpp"
#include "../include/HttpRequest.hpp"
#include <unistd.h>


Client::Client(int clientSocket) : _clientSocket(clientSocket), _HttpRequest(new HttpRequest()), _HttpResponse(new HttpResponse()), _CGI(NULL) {}

Client::~Client(){}

void Client::setSocket(int clientSocket){
	_clientSocket = clientSocket;
}

int Client::getSocket() const {
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
    char buf[READ_SIZE] = {0};
    int contentLength;

    // Try to receive data
    int received = recv(_clientSocket, buf, sizeof(buf), 0);

    if (received == 0) {
        throw std::runtime_error("Client closed connection");
    } 
	else if (received < 0) {
  	// Assume recv failed because no data is available (non-blocking)
        throw std::runtime_error("Error reading from socket");
        // return; // Gracefully exit if no data is available
    }

    // Append the data to the HTTP request buffer
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
}

void Client::writeToSocket() {
	unsigned long bytesToWrite = WRITE_SIZE;
    unsigned long bytesWritten = 0;

    if (bytesToWrite > _HttpResponse->getFullResponse().size() - _responseIndex) {
        bytesToWrite = _HttpResponse->getFullResponse().size() - _responseIndex;
    }
	//data + offset inputindex 
    bytesWritten = write(_clientSocket, _HttpResponse->getFullResponse().data() + _responseIndex, bytesToWrite);
    _responseIndex += bytesWritten;
}

void Client::closeConnection(EventPoll &eventPoll) {
    // Remove the client socket from EventPoll
    eventPoll.ToremovePollEventFd(getSocket(), POLLIN | POLLOUT);

    // Close the client socket
    close(getSocket());
}