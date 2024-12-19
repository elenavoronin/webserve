#include "../include/Client.hpp"

Client::Client(int clientSocket) : _clientSocket(clientSocket), _HttpRequest(new HttpRequest()), _HttpResponse(new HttpResponse()), _CGI(NULL) {}

/**
 * @brief client desctructor.
 *
 * @todo add deletes in here.
 */
Client::~Client(){}

/**
 * @brief Sets the file descriptor for the client's socket.
 *
 * @param clientSocket The file descriptor for the client's socket.
 */
void Client::setSocket(int clientSocket){
	_clientSocket = clientSocket;
}

/**
 * @brief Returns the file descriptor of the client's socket.
 *
 * @return The file descriptor of the client's socket.
 */
int Client::getSocket() const {
	return _clientSocket;
}

/**
 * @brief Sets the HttpRequest object for the client.
 *
 * @param httpRequest The HttpRequest object for the client.
 */
void Client::setHttpRequest(HttpRequest* httpRequest){
	_HttpRequest = httpRequest;
}

/**
 * @brief Sets the HttpResponse object for the client.
 *
 * @param httpResponse The HttpResponse object for the client.
 */
void Client::setHttpResponse(HttpResponse* httpResponse){
	_HttpResponse = httpResponse;
}

/**
 * @brief Retrieves the HttpRequest object associated with the client.
 *
 * @return A pointer to the client's HttpRequest object.
 */
HttpRequest* Client::getHttpRequest() const{
	return _HttpRequest;
}

/**
 * @brief Retrieves the HttpResponse object associated with the client.
 *
 * @return A pointer to the client's HttpResponse object.
 */
HttpResponse* Client::getHttpResponse() const{
	return _HttpResponse;
}

/**
 * @brief Returns the file descriptor of the read end of the CGI pipe.
 *
 * Returns -1 if the CGI object has not been initialized.
 *
 * @return The file descriptor of the read end of the CGI pipe or -1 if not initialized.
 */
int Client::getCgiRead(){
	if (this->_CGI == NULL)
		return -1;
	return this->_CGI->getReadFd();
}

/**
 * @brief Returns the file descriptor of the write end of the CGI pipe.
 *
 * Returns -1 if the CGI object has not been initialized.
 *
 * @return The file descriptor of the write end of the CGI pipe or -1 if not initialized.
 */
int Client::getCgiWrite(){
	if (this->_CGI == NULL)
		return -1;
	return this->_CGI->getWriteFd();
}

/**
 * @brief Starts the CGI process with the given HttpRequest.
 *
 * Initializes the CGI object by passing the HttpRequest object to its constructor.
 * If the CGI object has already been initialized, throws a std::runtime_error.
 *
 * @param request The HttpRequest object to initialize the CGI object with.
 */
void Client::startCgi(HttpRequest *request) {
	if (this->_CGI != NULL)
		throw std::runtime_error("already initialized");
	this->_CGI = new CGI(request);
}

/**
 * @brief Reads output from the CGI process.
 *
 * If the CGI object has not been initialized, throws a std::runtime_error.
 * Checks if the CGI process has finished writing output and either continues
 * reading or builds the HttpResponse object or throws an error.
 * If the CGI process has finished writing output, sends a signal to kill the
 * CGI process.
 */
void Client::readFromCgi() {
	if (this->_CGI == NULL)
		throw std::runtime_error("CGI not initialized");
	//check bool if done reading
		//decide continue reading
		//or build response
		//or error?
	//signal kill if done ?
}

/**
 * @brief Reads data from the client socket and appends it to the HTTP request buffer.
 *
 * If the client closes the connection, throws a std::runtime_error.
 * If there is an error reading from the socket, throws a std::runtime_error.
 * If the HTTP request headers have been fully received, processes the request using Server::processClientRequest.
 * If the HTTP request headers have not been fully received, continues to read from the socket.
 */
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

/**
 * @brief Writes the HTTP response to the client socket.
 *
 * @details Writes data from the _HttpResponse object to the client socket.
 *          The function will write data in chunks of WRITE_SIZE bytes until
 *          the response is fully written, or the end of the file is reached.
 *          The function assumes that the response is fully formed in the
 *          _HttpResponse object. The function does not handle errors
 *          associated with writing to the socket.
 */
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

/**
 * @brief Closes the client socket and removes it from the EventPoll.
 *
 * This method should be called when the client connection has been fully handled
 * and the client should be disconnected. It removes the client socket from the
 * EventPoll and closes the socket.
 *
 * @param eventPoll The EventPoll object to remove the client socket from.
 */
void Client::closeConnection(EventPoll &eventPoll) {
    // Remove the client socket from EventPoll
    eventPoll.ToremovePollEventFd(getSocket(), POLLIN | POLLOUT);

    // Close the client socket
    close(getSocket());
}