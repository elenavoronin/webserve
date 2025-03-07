#include "../include/Client.hpp"

Client::Client(int clientSocket, EventPoll& eventPoll) : 
    _clientSocket(clientSocket), 
    _HttpRequest(new HttpRequest()), 
    _HttpResponse(new HttpResponse()), 
    _CGI(nullptr), 
    _eventPoll(&eventPoll),
    _responseIndex(0){
	_start_time = std::chrono::system_clock::now();
}

/**
 * @brief client desctructor.
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
      _CGI(copy._CGI ? new CGI(*copy._CGI) : nullptr),
      _eventPoll(copy._eventPoll),
      _responseIndex(copy._responseIndex),
      _start_time(copy._start_time) {}

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
    if (this == &copy) return *this;

    delete _HttpRequest;
    delete _HttpResponse;
    delete _CGI;

    _clientSocket = copy._clientSocket; 
    _HttpRequest = copy._HttpRequest ? new HttpRequest(*copy._HttpRequest) : nullptr;
    _HttpResponse = copy._HttpResponse ? new HttpResponse(*copy._HttpResponse) : nullptr;
    _CGI = copy._CGI ? new CGI(*copy._CGI) : nullptr;
    _eventPoll = copy._eventPoll; 
    _responseIndex = copy._responseIndex;
    _start_time = copy._start_time;

    return *this;
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
 * @brief Retrieves the CGI object associated with the client.
 *
 * Returns a pointer to the CGI object associated with the client or NULL if no
 * CGI object has been initialized.
 *
 * @return A pointer to the client's CGI object or NULL if not initialized.
 */
CGI* Client::getCGI() const {
    return _CGI;
}

/**
 * @brief Starts the CGI process with the given HttpRequest.
 *
 * Initializes the CGI object by passing the HttpRequest object to its constructor.
 * If the CGI object has already been initialized, throws a std::runtime_error.
 *
 * @param request The HttpRequest object to initialize the CGI object with.
 */
void Client::startCgi(HttpRequest *request){
	if (this->_CGI != NULL)
    {
		throw std::runtime_error("already initialized");
        return ;
    }
    

	this->_CGI = new CGI(request);
    
    
    _eventPoll->addPollFdEventQueue(_CGI->getReadFd(), POLLIN);
    _eventPoll->addPollFdEventQueue(_CGI->getWriteFd(), POLLOUT);
    _eventPoll->ToremovePollEventFd(_clientSocket, POLLIN);

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
        _CGI->readCgiOutput();
        if (_CGI->isCgiComplete()) {
            _HttpResponse->setFullResponse(_CGI->getCgiOutput());
            _eventPoll->ToremovePollEventFd(_CGI->getReadFd(), POLLIN);
            _eventPoll->ToremovePollEventFd(_CGI->getWriteFd(), POLLOUT);
            _eventPoll->addPollFdEventQueue(_clientSocket, POLLOUT);

            kill(_CGI->getPid(), SIGTERM);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error while reading from CGI: " << e.what() << std::endl;
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
void Client::readFromSocket(Server *server, defaultServer defaultS, std::vector<defaultServer> servers) {
    if (!_HttpRequest) {
        throw std::runtime_error("_HttpRequest is null. Possible use-after-free.");
    }
    
    char buf[READ_SIZE] = {0};
    int received = recv(_clientSocket, buf, sizeof(buf), 0);
    if (received == 0) {
        return;
    }
    if (received < 0) {
        throw std::runtime_error("Error reading from socket");
    }

    _HttpRequest->getStrReceived().append(buf, received);
    size_t headerEnd = _HttpRequest->getStrReceived().find("\r\n\r\n");

    if (headerEnd != std::string::npos) {
        int contentLength = _HttpRequest->findContentLength(_HttpRequest->getStrReceived());
        
        if (_HttpRequest->getStrReceived().length() >= headerEnd + 4 + contentLength) {
            _HttpRequest->setHeaderReceived(true);
            _HttpRequest->parseHeaders(_HttpRequest->getStrReceived());
        } 
        else {
            return;
        }
    }

    if (_HttpRequest->isHeaderReceived()) {
        _HttpRequest->parseBody(_HttpRequest->getStrReceived());
        server->processClientRequest(*this, _HttpRequest->getStrReceived(), _HttpRequest, defaultS, servers);
        _HttpRequest->reset();
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
    unsigned long bytesWritten = 0;

    if (bytesToWrite > _HttpResponse->getFullResponse().size() - _responseIndex) {
        bytesToWrite = _HttpResponse->getFullResponse().size() - _responseIndex;
    }
    bytesWritten = write(_clientSocket, _HttpResponse->getFullResponse().data() + _responseIndex, bytesToWrite);

    if (bytesWritten < 0) {
        throw std::runtime_error("Error writing to socket");
    }

    if (bytesWritten > 0) {
        _responseIndex += bytesWritten;
    }

    if (_responseIndex >= _HttpResponse->getFullResponse().size()) {
        _eventPoll->ToremovePollEventFd(_clientSocket, POLLOUT);
        return 1;
    }
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
void Client::closeConnection(EventPoll& eventPoll, int currentPollFd) {

    if (_HttpResponse && _responseIndex < _HttpResponse->getFullResponse().size()) {
        eventPoll.addPollFdEventQueue(_clientSocket, POLLOUT);
        return;
    }
    if (currentPollFd != 0)
    {
        eventPoll.ToremovePollEventFd(currentPollFd, POLLIN | POLLOUT);
    }

    if (_clientSocket >= 0) {
        eventPoll.ToremovePollEventFd(_clientSocket, POLLIN | POLLOUT);
        eventPoll.ToremovePollEventFd(_clientSocket, POLLIN | POLLOUT);
        close(_clientSocket);
        return;
    }
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
    _CGI->writeCgiInput();
}

/**
 * @brief Removes a file descriptor and event type from the EventPoll's queue.
 *
 * This function schedules the removal of a specified file descriptor and event type
 * from the EventPoll's queue. The removal is executed in the updateEventList() function.
 *
 * @param fd The file descriptor to remove from the EventPoll.
 * @param eventType The event type (such as POLLIN or POLLOUT) to remove.
 */

void Client::addToEventPollRemove(int fd, int eventType) {
    _eventPoll->ToremovePollEventFd(fd, eventType);
}

/**
 * @brief Adds a file descriptor and event type to the EventPoll's queue.
 *
 * This function adds a file descriptor and event type to the EventPoll's queue
 * to be monitored for events. The addition is done in the updateEventList()
 * function.
 *
 * @param fd The file descriptor to add to the EventPoll.
 * @param eventType The event type (such as POLLIN or POLLOUT) to add.
 */
void Client::addToEventPollQueue(int fd, int eventType) {
    _eventPoll->addPollFdEventQueue(fd, eventType);
}

/**
 * @brief Sets the start time for the client.
 *
 * This function assigns the provided time point to the client's start time,
 * which can be used to track the duration or timeout of a client connection.
 *
 * @param start_time The time point to set as the client's start time.
 */

void Client::setStartTime(std::chrono::system_clock::time_point start_time){
	_start_time = start_time;
}

/**
 * @brief Retrieves the client's start time.
 *
 * This function returns the time point set as the client's start time.
 *
 * @return The client's start time.
 */
std::chrono::system_clock::time_point  Client::getStartTime() const{
	return _start_time;
}