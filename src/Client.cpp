#include "../include/Client.hpp"

Client::Client(int clientSocket, EventPoll& eventPoll) : 
    _clientSocket(clientSocket), 
    _HttpRequest(new HttpRequest()), 
    _HttpResponse(new HttpResponse()), 
    _CGI(nullptr), 
    _eventPoll(&eventPoll),
    _responseIndex(0){}

/**
 * @brief client desctructor.
 *
 * @todo add deletes in here.
 */
Client::~Client(){
    delete _HttpRequest;
    delete _HttpResponse;
    delete _CGI;
    _HttpRequest = nullptr;
    _HttpResponse = nullptr;
    _CGI = nullptr;
}

/**
 * @brief Copy constructor for the Client class.
 *
 * This constructor initializes a new Client object by performing a deep copy
 * of the provided Client object. It copies the client's socket, HTTP request
 * and response objects, and other relevant data members. If the original
 * Client has associated HTTP request or response objects, new copies of those
 * objects are created to ensure independent ownership.
 *
 * @param copy The Client object to copy from.
 */
Client::Client(const Client& copy)     
    : _clientSocket(copy._clientSocket), 
      _HttpRequest(copy._HttpRequest ? new HttpRequest(*copy._HttpRequest) : nullptr),
      _HttpResponse(copy._HttpResponse ? new HttpResponse(*copy._HttpResponse) : nullptr),
      _CGI(nullptr),
      _eventPoll(copy._eventPoll), // Initialize the reference
      _responseIndex(copy._responseIndex) {}


/**
 * @brief Copy assignment operator for the Client class.
 *
 * This function performs a deep copy of the provided Client object. It copies
 * the client's socket, HTTP request and response objects, and other relevant
 * data members. If the original Client has associated HTTP request or response
 * objects, new copies of those objects are created to ensure independent
 * ownership.
 *
 * @param copy The Client object to copy from.
 * @return A reference to the copied Client object.
 */
Client& Client::operator=(const Client& copy) {    
    if (this == &copy) return *this; // Handle self-assignment

    // Cleanup existing resources
    delete _HttpRequest;
    delete _HttpResponse;
    delete _CGI;

    // Copy resources
    _clientSocket = copy._clientSocket; // this messes things up
    _HttpRequest = copy._HttpRequest ? new HttpRequest(*copy._HttpRequest) : nullptr;
    _HttpResponse = copy._HttpResponse ? new HttpResponse(*copy._HttpResponse) : nullptr;
    _CGI = nullptr; // Reset CGI in the assigned instance
    _eventPoll = copy._eventPoll; // References must remain valid
    _responseIndex = copy._responseIndex;

    return *this;
}

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
    _eventPoll->addPollFdEventQueue(_CGI->getReadFd(), POLLIN);
    _eventPoll->addPollFdEventQueue(_CGI->getWriteFd(), POLLOUT);

    std::cerr   << "CGI process started. Read FD: " << _CGI->getReadFd()
                << ", Write FD: " << _CGI->getWriteFd() << std::endl;
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
    if (!_CGI) {
        throw std::runtime_error("CGI object is not initialized.");
    }

    try {
        // Read data from the CGI process
        _CGI->readCgiOutput();

        // Check if headers have been sent
        if (!_CGI->areHeadersSent()) {
            // Send HTTP headers
            HttpResponse response;
            response.setStatus(200, "OK");
            response.setHeader("Content-Type", "text/html");
            response.setHeader("Transfer-Encoding", "chunked");

            std::string headers = response.getHeadersOnly();
            _CGI->markHeadersSent();
        }

        // Check if the CGI process is done writing output
        if (_CGI->isCgiComplete()) {
            // Send the final chunk and terminate the CGI process
            send(_clientSocket, "0\r\n\r\n", 5, 0);
            kill(_CGI->getPid(), SIGTERM); // Send signal to terminate the process
            // _CGI->markCgiComplete();
            prepareFileResponse();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error while reading from CGI: " << e.what() << std::endl;
        // Handle cleanup or error response
        // close(_clientSocket); // Optionally close the connection
    }
}

/**
 * @brief Reads data from the client socket and appends it to the HTTP request buffer.
 * 
 * @param server Pointer to the Server object.
 * @param defaultServer Reference to the default Server object.
 *
 * If the client closes the connection, throws a std::runtime_error.
 * If there is an error reading from the socket, throws a std::runtime_error.
 * If the HTTP request headers have been fully received, processes the request using Server::processClientRequest.
 * If the HTTP request headers have not been fully received, continues to read from the socket.
 */
void Client::readFromSocket(Server *server, Server &defaultServer) {
    if (!_HttpRequest) {
        throw std::runtime_error("_HttpRequest is null. Possible use-after-free.");
    }
    
    char buf[READ_SIZE] = {0};
    int contentLength;

    // Try to receive data
    int received = recv(_clientSocket, buf, sizeof(buf), 0);

    if (received == 0) {
        throw std::runtime_error("Client closed connection");
    } 
	else if (received < 0) {
        throw std::runtime_error("Error reading from socket");
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
        server->processClientRequest(*this, _HttpRequest->getStrReceived(), _HttpRequest, defaultServer);
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
int Client::writeToSocket() {
    unsigned long bytesToWrite = WRITE_SIZE;
    // unsigned long bytesToWrite = 10000;
    unsigned long bytesWritten = 0;

    if (bytesToWrite > _HttpResponse->getFullResponse().size() - _responseIndex) {
        bytesToWrite = _HttpResponse->getFullResponse().size() - _responseIndex;
    }
    bytesWritten = write(_clientSocket, _HttpResponse->getFullResponse().data() + _responseIndex, bytesToWrite);

    if (bytesWritten > 0) {
        _responseIndex += bytesWritten;
    }

    if (_responseIndex >= _HttpResponse->getFullResponse().size()) {
        _eventPoll->ToremovePollEventFd(_clientSocket, POLLOUT);
        return 1;
    }
    // std::cout << "STATUSSSS: " << _HttpResponse->getStatusCode() << std::endl;
    return 0;
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
    // Remove socket from event poll
    eventPoll.ToremovePollEventFd(_clientSocket, POLLIN | POLLOUT);

    if (_CGI) {
        eventPoll.ToremovePollEventFd(_CGI->getReadFd(), POLLIN);
        delete _CGI;
        _CGI = nullptr;
    }
    
    // Close the client socket
    if (_clientSocket >= 0) {
        close(_clientSocket);
    }
}

/**
 * @brief Prepares a file response for the client by building the response and adding POLLOUT to the EventPoll.
 *
 * This method should be called when the client connection has been fully handled
 * and the file response should be sent. It builds the HTTP response using the
 * HttpResponse object and adds POLLOUT to the EventPoll, so that the response
 * can be written to the client socket when it is ready.
 * 
 * @todo add response status code
 * @todo add timeout?
 */
void Client::prepareFileResponse() {
    std::string requestedFile = _HttpRequest->getFullPath();
    //read file
    std::ifstream file(requestedFile);
    if (!file.is_open()) {
        throw std::runtime_error("File not found 1: " + requestedFile);
        return ;
        // Handle 404 response
    } else {
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        // Send the file content as response
        _HttpResponse->setBody(buffer.str());
        _HttpResponse->setHeader("Content-Type", "text/html");
        // _HttpResponse->setStatusCode(200); //replace by actual status code
    }

    _HttpResponse->buildResponse();
    //std::cout << "Preparing file response for client socket: " << _clientSocket << std::endl;
    _eventPoll->ToremovePollEventFd(_clientSocket, POLLIN);
    _eventPoll->addPollFdEventQueue(_clientSocket, POLLOUT);
    //std::cout << "Added POLLOUT for client socket: " << _clientSocket << std::endl;

}



/**
 * @brief Writes the client's input to the CGI process via the pipe.
 *
 * This function should be called when the client connection has been fully handled
 * and the input should be written to the CGI process. It delegates the write operation
 * to the CGI object.
 *
 * @throws std::runtime_error if the CGI object is not initialized.
 */
void Client::writeToCgi() {
    if (!_CGI) {
        throw std::runtime_error("CGI object is not initialized.");
    }

    // Delegate the write operation to the CGI object
    _CGI->writeCgiInput();
}
