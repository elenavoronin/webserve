#include "../include/Server.hpp"

Server::Server(){}

Server::~Server(){}

/**
 * @brief Initializes and returns a listener socket for the server.
 *
 * This function sets up a TCP listener socket using the server's configured port.
 * It configures address information, creates the socket, sets socket options, 
 * binds the socket to the specified address, and begins listening for incoming
 * connections. The function returns the file descriptor for the created listener
 * socket on success, or exits the program if binding fails for all addresses.
 * 
 * status: Result of getaddrinfo(), checks if address info is valid
 * hints: addrinfo structure with settings for creating socket
 * servinfo: Linked list of address structures
 * newConnect: Pointer to iterate through servinfo to create a socket
 * serverSocket: File descriptor for server socket
 * opt: Option value for setsockopt() to allow address reuse
 *
 * @return The file descriptor for the created listener socket on success, 
 *         or -1 if an error occurs during listening setup.
 */
int Server::getListenerSocket(){
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;
	struct addrinfo *newConnect;

	int serverSocket;
	int opt = 1;
	memset(&hints, 0, sizeof hints); //is used to clear out the hints struct. Then, we fill in some details:
	hints.ai_family = AF_UNSPEC; // allows either IPv4 or IPv6.
	hints.ai_socktype = SOCK_STREAM; // tells the system to use TCP
	hints.ai_flags = AI_PASSIVE; //makes the program automatically fill in the IP 
	if ((status = getaddrinfo(NULL, getPortStr().c_str(), &hints, &servinfo)) != 0){
		//std::cout << "Error get Address information" << std::endl;
		return 1;
	}
	for (newConnect = servinfo; newConnect != NULL; newConnect= newConnect->ai_next){
		if ((serverSocket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1){ //creates a socket
			// //std::cout << "Create server socket " << serverSocket << std::endl;
			continue;
		}
		setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //allows the program to reuse the address
		if (bind(serverSocket, newConnect->ai_addr, newConnect->ai_addrlen) == -1){ //associates the socket with an address (IP and port).
			//std::cout << "Bind error" << std::endl;
			close(serverSocket);
			continue;
		}
		break;
	}
	freeaddrinfo(servinfo);
	if (newConnect == NULL) //If no address was successfully bound, the program exits with an error.
		exit(1);
	if (listen(serverSocket, BACKLOG) == -1) //tells the socket to listen for incoming connections
		return -1;
	// //std::cout << "serverSocket " << serverSocket << std::endl;
	return serverSocket;
}

/**
 * @brief Reports that the server is ready to accept connections
 *
 * This function sets up a listening socket and adds it to the EventPoll
 * so that it can be monitored for incoming connections.
 *
 * @param eventPoll The EventPoll to add the listening socket to
 * @return The file descriptor of the listening socket
 * @todo check that listener_fd is properly initialized and not overwritten elsewhere
 */
int Server::reportReady(EventPoll &eventPoll){
	int listener = getListenerSocket(); // Set up and get a listening socket
	if (listener == -1){
		throw std::runtime_error("Error get listener socket");
	}
    // Add the listener to EventPoll
    eventPoll.addPollFdEventQueue(listener, POLLIN);
    return listener; //TODO do we need to treturn listeren or should listener be a private variable?
}

/**
 * @brief Handle a new connection and add it to the list of clients and the
 * EventPoll
 *
 * This function accepts a new connection and adds it to the clients vector.
 * It also adds the new client's file descriptor to the EventPoll so that it
 * can be monitored for incoming data.
 *
 * @param eventPoll The EventPoll to add the new client to
 * @todo replace perror with throw
 */
void Server::handleNewConnection(EventPoll &eventPoll){

	int new_fd = accept(_listener_fd, nullptr, nullptr);
    if (new_fd == -1) {
        perror("Error accepting new connection");
        return;
    }
	
    std::cout << "OPEN!!! New connection accepted on fd: " << new_fd << std::endl;

	Client newClient(new_fd, eventPoll);
    // Add the new client directly to the clients vector
    _clients.push_back(newClient);
    eventPoll.addPollFdEventQueue(new_fd, POLLIN);

    //std::cout << "Added new client to EventPoll with fd: " << new_fd << std::endl;
}

/**
 * @brief Handle events on a file descriptor
 *
 * This function takes an index into the EventPoll's list of pollfds, and
 * handles the event(s) associated with that file descriptor. It first
 * finds the Client associated with the file descriptor, and then
 * processes the event(s) according to the type of event.
 *
 * @param eventPoll The EventPoll object containing the pollfds
 * @param i The index into the pollfds vector
 * @param defaultServer The default Server object
 * @todo  divide into smaller functions doing one thing
 * @todo throw instead of error or cout
 */
void Server::handlePollEvent(EventPoll &eventPoll, int i, Server& defaultServer) {
    Client *client = nullptr;
    pollfd &currentPollFd = eventPoll.getPollEventFd()[i];
    int event_fd = currentPollFd.fd;

    // Find the client associated with the file descriptor
    for (auto &c : _clients) {
        if (c.getSocket() == event_fd || 
			c.getCgiRead() == event_fd || 
			c.getCgiWrite() == event_fd) {
            client = &c;
            break;
        }
    }

    if (!client) {
        std::cerr << "Client not found for fd: " << event_fd << std::endl;
		    //client->closeConnection(eventPoll);
		    eraseClient(event_fd);
        return;
    }

    // Handle readable events
    if (currentPollFd.revents & POLLIN) {
		// std::cout << "POLLIN" << std::endl;
        try {
            if (event_fd != client->getSocket() && event_fd == client->getCgiRead()) {
                client->readFromCgi();
            } else {
                client->readFromSocket(this, defaultServer);
            }
        } catch (const std::runtime_error &e) {
            std::cerr << "Read error: " << e.what() << std::endl;
            client->closeConnection(eventPoll, currentPollFd.fd);
			eraseClient(event_fd);
        }
    }
    // Handle writable events
    if (currentPollFd.revents & POLLOUT) {

        try {
            if (event_fd != client->getSocket() && event_fd == client->getCgiWrite()) {
                client->writeToCgi();
            } else {
                if (client->writeToSocket() > 0) {
					client->closeConnection(eventPoll, currentPollFd.fd);
					eraseClient(event_fd);
				}
            }
        } catch (const std::runtime_error &e) {
            std::cerr << "Write error: " << e.what() << std::endl;
            client->closeConnection(eventPoll, currentPollFd.fd);
			eraseClient(event_fd);
        }
    }

    // Handle hangup or disconnection events
    if (currentPollFd.revents & (POLLHUP | POLLRDHUP)) {
		// std::cout << "does this happen" << std::endl;
        client->closeConnection(eventPoll, currentPollFd.fd);
		eraseClient(event_fd);
    }
}


/**
 * @brief Checks and updates the server's root based on the given path.
 *
 * This function compares a given path with the server's index. If they
 * match, the function returns immediately. Otherwise, it iterates over
 * the server's locations to find a matching path. If a matching location
 * is found and it is not empty, the server's root is updated to the root
 * defined in the location.
 *
 * @param path The path to check against the server's locations.
 * @todo figure out when to reset server information to default
 */
void Server::checkLocations(std::string path, Server &defaultServer) {
    if (path == this->getIndex()) {
		this->setRoot(defaultServer.getRoot());
		this->setIndex(defaultServer.getIndex());
		this->setPortString(defaultServer.getPortStr());
		this->setAllowedMethods(defaultServer.getAllowedMethods());
		this->setAutoindex(defaultServer.getAutoindex());
		this->setMaxBodySize(defaultServer.getMaxBodySize());
		this->setUploadStore(defaultServer.getUploadStore());
		this->setErrorPage(defaultServer.getErrorPage());
        this->setRedirect(std::to_string((defaultServer.getRedirect().first)), defaultServer.getRedirect().second);
		return;
	}
	for (const auto& location : this->getLocations()) {
		if (path == location.first) {
			if (!location.second.empty()) {
				Location loc = location.second[0];
				if (!loc.getRoot().empty())
					this->setRoot(loc.getRoot());
				if (!loc.getIndex().empty())
					this->setIndex(loc.getIndex());
				if (!loc.getAllowedMethods().empty())
					this->setAllowedMethods(loc.getAllowedMethods());
				if (loc.getAutoindex())
					this->setAutoindex(loc.getAutoindex());
				if (loc.getMaxBodySize() != 0)
					this->setMaxBodySize(loc.getMaxBodySize());
				if (!loc.getErrorPages().empty())
					this->setErrorPage(loc.getErrorPages());
                if (loc.getRedirect().first != 0)
                    this->setRedirect(std::to_string((defaultServer.getRedirect().first)), defaultServer.getRedirect().second);
				return;
			}
		}
	}
}

/**
 * @brief Process an HTTP request received from a client.
 *
 * This function processes an HTTP request string and takes the appropriate
 * action based on the request method and path. It returns an HTTP status code
 * indicating the result of the request processing.
 *
 * If the request method is not allowed for the given path, a 405 Method Not
 * Allowed response is sent to the client. If the request path does not match
 * any of the server's locations, a 404 Not Found response is sent to the
 * client. If there is an error parsing the request, a 400 Bad Request response
 * is sent to the client.
 *
 * @param client The client object associated with the request.
 * @param request The HTTP request string received from the client.
 * @param HttpRequest The HttpRequest object associated with the client.
 * @return The HTTP status code indicating the result of the request processing.
 */
int Server::processClientRequest(Client &client, const std::string& request, HttpRequest* HttpRequest, Server &defaultServer) {
	HttpRequest->readRequest(request);

	std::string method = HttpRequest->getMethod();
    std::string path = HttpRequest->getPath();
    std::string version = HttpRequest->getVersion();

	checkLocations(path, defaultServer);
    if (getRedirect().first != 0)
        return handleRedirect(client, *HttpRequest);
	
    int status = validateRequest(method, version);
	if (status != 200) {
		sendFileResponse(client.getSocket(), "www/html/500.html", status);  //change to a config ones?
		return status;
	}
    // if (redirect) // TODO handle redirect before handling any other methods

	if (method == "GET" && std::find(this->_allowedMethods.begin(), this->_allowedMethods.end(), "GET") != this->_allowedMethods.end())
		return handleGetRequest(client, HttpRequest); //?? what locations should be passed
	if (method == "POST" && std::find(this->_allowedMethods.begin(), this->_allowedMethods.end(), "POST") != this->_allowedMethods.end())
		return handlePostRequest(client, HttpRequest);
	if (method == "DELETE" && std::find(this->_allowedMethods.begin(), this->_allowedMethods.end(), "DELETE") != this->_allowedMethods.end())
		return handleDeleteRequest(client, HttpRequest);
	HttpResponse response;
	response.buildResponse();
	return 0;
}

/**
 * @brief Handles an HTTP GET request from a client.
 *
 * This function takes a request path and determines how to handle the GET request.
 * If the path is a directory, the function sends the contents of the directory
 * as the response body. If the path is a file, the function reads the contents
 * of the file and sends it as the response body. If the path does not exist, a
 * 404 Not Found response is sent with a simple HTML page indicating that the
 * file was not found.
 *
 * @param client The client object associated with the request.
 * @param path The path of the GET request.
 * @param request The HttpRequest object associated with the client.
 * @return The HTTP status code indicating the result of the request processing.
 */
int Server::handleGetRequest(Client &client, HttpRequest* request) {
	HttpResponse response;
	std::string filepath = this->getRoot() + request->getPath();
	request->setFullPath(filepath);
    std::cout << "filepath: " << filepath << std::endl;

	if (request->getPath() == "/") {
		filepath = this->getRoot() + '/' + this->getIndex();
		request->setFullPath(filepath);
	}
    size_t position = filepath.find("/cgi-bin"); //todo change this to config
    if (position != std::string::npos) { 
		request->setFullPath(filepath);
		client.startCgi(request);
		return 0;
	}
	try {
        std::ifstream file(filepath);
        if (!file) {
            throw std::runtime_error(" " + filepath);
        }

        // Proceed with sending the file response
        client.prepareFileResponse("");
		client.sendData(client.getHttpResponse()->getFullResponse());
		response.buildResponse();

        return 200;

    } catch (const std::runtime_error& e) {
        std::string errorPagePath = "www/html/404.html"; // TODO fix this to not be hardcoded
        std::ifstream errorFile(errorPagePath, std::ios::binary);
        std::string errorContent;

        if (errorFile.is_open()) {
            std::stringstream buffer;
            buffer << errorFile.rdbuf();
            errorContent = buffer.str();
            errorFile.close();
        } else {
            // Fallback if the 404 page itself is missing
            errorContent = "<html><body><h1>404 - Page Not Found</h1></body></html>";
        }
        client.prepareFileResponse(errorContent);
		client.sendData(client.getHttpResponse()->getFullResponse());
		response.buildResponse();
        return 404;
	}
}

/**
 * @brief Send a file response to the client.
 *
 * This function reads the contents of the specified file and sends it as the body
 * of an HTTP response to the client. If the file does not exist, a 404 Not Found
 * response is sent with a simple HTML page indicating that the file was not found.
 *
 * @param clientSocket The socket to send the response to.
 * @param filepath The path to the file to send.
 * @param statusCode The HTTP status code to send in the response.
 * @todo check sendHeaders() for passing content type not hard coded
 */
void Server::sendFileResponse(int clientSocket, const std::string& filepath, int statusCode) {
	std::string fileContent = readFileContent(filepath);
	if (fileContent.empty()) {
		sendHeaders(clientSocket, 404, "text/html");
		sendBody(clientSocket, "<html><body>404 - File Not Found</body></html>"); //TODO change error pages to server stuff
	} else {
		sendHeaders(clientSocket, statusCode, "text/html");
		sendBody(clientSocket, fileContent);
	}
	close(clientSocket);
}

/**
 * @brief Read the contents of a file into a string.
 *
 * This function reads the contents of a file specified by the given filepath
 * and returns the contents as a string. If the file does not exist, an error
 * message is printed to stderr and an empty string is returned.
 *
 * @param filepath The path to the file to read.
 * @return The contents of the file as a string.
 */
std::string Server::readFileContent(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file) {
        std::cerr << "Error: File not found 2: " << filepath << std::endl;
        return "";
    }
    std::ostringstream buffer;
    buffer << file.rdbuf(); //read file by bytes, go back to poll, check if finished reading
	//request->_readyToSendBack = true;
	//std::cout.flush();
    return buffer.str();
}

/**
 * @brief Send HTTP headers to the client.
 *
 * This function sends the HTTP headers (status line and content type) to
 * the client. The status line is constructed with the provided status code
 * and the corresponding status message. The "Content-Type" header is set to
 * the provided content type. If no content type is provided, it defaults
 * to "text/html".
 *
 * @param clientSocket The socket to send the headers to.
 * @param statusCode The HTTP status code to send.
 * @param contentType The content type to send (optional, defaults to "text/html").
 */
void Server::sendHeaders(int clientSocket, int statusCode, const std::string& contentType = "text/html") {
    std::string statusMessage = getStatusMessage(statusCode);
    std::ostringstream headers;
    headers << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    // For persistent connections (such as in HTTP/1.1), you would leave the client in the pfds list to handle further requests.
    // For non-persistent connections (such as in HTTP/1.0), it's appropriate to remove the client after processing the request.
    headers << "Content-Type: " << contentType << "\r\n\r\n"; // Default is text/html
    std::string headersStr = headers.str();
    send(clientSocket, headersStr.c_str(), headersStr.size(), 0);
}

/**
 * @brief Send the body of the HTTP response to the client.
 *
 * This function sends the contents of the provided string as the body
 * of the HTTP response to the client.
 *
 * @param clientSocket The socket to send the body to.
 * @param body The body content to send.
 */
void Server::sendBody(int clientSocket, const std::string& body) {
    send(clientSocket, body.c_str(), body.size(), 0);
}

/**
 * @brief Validates the HTTP method and version of the request.
 * 
 * This function checks if the provided HTTP method is one of the
 * supported methods ("GET", "POST", "DELETE") and if the version
 * is "HTTP/1.1". If the method is not supported, it returns a 405
 * status code indicating "Method Not Allowed". If the version is
 * not "HTTP/1.1", it returns a 400 status code for "Bad Request".
 * On successful validation, it returns a 200 status code for "OK".
 * 
 * @param method The HTTP method to validate.
 * @param version The HTTP version to validate.
 * @return The HTTP status code indicating the result of the validation.
 */
int Server::validateRequest(const std::string& method, const std::string& version) {
	if (method != "GET" && method != "POST" && method != "DELETE") {
		std::cerr << "Error: Unsupported HTTP method." << std::endl;
		return 405; // Method Not Allowed
	}

	if (version != "HTTP/1.1") {
		std::cerr << "Error: Invalid HTTP version. Only HTTP/1.1 is supported." << std::endl;
		return 400;
	}
	return 200;
}


/**
 * @brief Handles an HTTP DELETE request from a client.
 * 
 * This function processes a DELETE request by determining the path of the resource
 * to be deleted from the request. It checks if the resource exists and is writable,
 * then attempts to delete it. If successful, a 200 OK response is sent back to the client.
 * If the resource does not exist or any error occurs during deletion, an appropriate
 * error response is sent, such as a 404 Not Found or 500 Internal Server Error.
 * @param client The client object associated with the request.
 * @param request The HttpRequest object associated with the client.
 * @todo
 * 
 * @return The HTTP status code indicating the result of the request processing.
 */
int Server::handleDeleteRequest(Client &client, HttpRequest* request) {
    HttpResponse response;
    try {
        std::string pathToDelete =  request->getPathToDelete(request->getRawRequest());
        if (pathToDelete.empty()) {
            response.setStatus(404, getStatusMessage(404));
            response.setHeader("Content-Type", "text/plain");
            response.setBody("Content to be deleted not found");
            response.buildResponse();
            return 404;
        }
      
       // Check if the file exists
       std::ifstream file(pathToDelete);
        if (!file.is_open()) {
            throw std::runtime_error("File not found: " + pathToDelete);
        }

        // Check if the file is accessible and writable
        if (access(pathToDelete.c_str(), W_OK) != 0) {
            throw std::runtime_error("File cannot be deleted: " + pathToDelete);
        }

        // Delete the file
        if (std::remove(pathToDelete.c_str()) != 0) {
            throw std::runtime_error("Error deleting file '" + pathToDelete);
        }
    
        // Set success response
        response.setStatus(200, getStatusMessage(200));
        response.setHeader("Content-Type", "text/plain");
        response.setBody("Data deleted successfully.");
        response.buildResponse();
        std::cout << "SUCCESS DELETED FILE" << std::endl;
        // Send the response
        client.sendData(response.getFullResponse());
        return 200;

    } catch (const std::exception &e) {
        // Handle errors and return a 500 Internal Server Error
        std::cerr << "Error handling Delete request: " << e.what() << std::endl;

        response.setStatus(500, getStatusMessage(500));
        response.setHeader("Content-Type", "text/plain");
        response.setBody("Error processing the DELETE request.");
        response.buildResponse();

        client.sendData(response.getFullResponse());
        return 500;
    }
}


/**
 * @brief Handles an HTTP POST request from a client.
 *
 * This function processes the body of a POST request, validates the
 * Content-Length header, and ensures the received data matches the expected
 * content length. If the validation is successful, it processes the data
 * by saving it to a file or performing other actions as needed. Upon
 * successful processing, a 201 Created response is sent to the client.
 * In case of any errors, a 500 Internal Server Error response is sent.
 *
 * @param client The client object associated with the request.
 * @param request The HttpRequest object containing the details of the POST request.
 * @return The HTTP status code indicating the result of the request processing.
 * @todo check if body size exceeds a predefined limit (error 413 Request Too Large)
 * @todo Error (400 Bad Request): If there was a problem with the data.
 */

int Server::handlePostRequest(Client &client, HttpRequest* request) {
 	HttpResponse response;
    try {
        // Get the request body
        std::string requestBody = request->getBody();
        // Validate the Content-Length header
		// std::cout << "body of http request: " << requestBody << std::endl;
        std::string contentLengthHeader = request->getHeader("Content-Length");
		if (contentLengthHeader.empty()) {
    		throw std::runtime_error("Missing Content-Length header in POST request");
		}
        if (contentLengthHeader.empty()) {
            throw std::runtime_error("Missing Content-Length header in POST request");
        }

        size_t contentLength = std::stoul(contentLengthHeader);
        if (requestBody.size() != contentLength) {
            throw std::runtime_error("Content-Length mismatch: Received " +
                                     std::to_string(requestBody.size()) +
                                     ", expected " + contentLengthHeader);
        }

        // Example: Process the data (e.g., save to file, database, etc.)
        std::string filename = "data_upload.txt";
        std::string savePath = getUploadStore() + filename;
        std::cout << "save path: " << savePath << std::endl;
        std::ofstream outFile(savePath, std::ios::app);
        if (!outFile.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + savePath);
        }
        outFile << requestBody << std::endl;
        outFile.close();

        // Set success response
        response.setStatus(201, getStatusMessage(201));
        response.setHeader("Content-Type", "text/plain");
        response.setBody("Data received and stored successfully.");
        response.buildResponse();

        // Send the response
        client.sendData(response.getFullResponse());
        return 201;

    } catch (const std::exception &e) {
        // Handle errors and return a 500 Internal Server Error
        std::cerr << "Error handling POST request: " << e.what() << std::endl;

        response.setStatus(500, getStatusMessage(500));
        response.setHeader("Content-Type", "text/plain");
        response.setBody("Error processing the POST request.");
        response.buildResponse();

        client.sendData(response.getFullResponse());
        return 500;
    }
}


/**
 * @brief Handles an HTTP redirect request.
 *
 * @details This function will handle any redirects that are configured on the server.
 *          It will set the appropriate status code and location for the redirect,
 *          and provide an optional body if desired.
 *
 * @param client The client connection to send the response to
 * @param request The HTTP request (not used currently)
 * @todo I still need to parse the request
 *
 * @return The status code of the response
 */
int Server::handleRedirect(Client& client, HttpRequest& request) {
     HttpResponse response;
    std::cout << " I AM DOING REDICRECTION" << std::endl;
    (void)request;// TODO I still need to parse the request
    try {
        // Set the redirection status code and location
        if (getRedirect().first == 301)
            response.setStatus(301, getStatusMessage(301));
        else
            response.setStatus(302, getStatusMessage(302));
        response.setHeader("Location", getRedirect().second);

        // Provide an optional body for the redirection response
        response.setHeader("Content-Type", "text/html");
        std::string body = "<html><body><h1>Redirecting...</h1>"
                           "<p>If you are not redirected automatically, "
                           "<a href=\"" + getRedirect().second + "\">click here</a>.</p></body></html>";
        response.setBody(body);

        // Build and send the response
        response.buildResponse();
        client.sendData(response.getFullResponse());
        std::cout << "Redirected to: " << getRedirect().second << " with status code: " << getRedirect().first << std::endl;

        return getRedirect().first;

    } catch (const std::exception &e) {
        std::cerr << "Error handling redirection request: " << e.what() << std::endl;

        // Handle errors and send a 500 Internal Server Error response
        response.setStatus(500, getStatusMessage(500));
        response.setHeader("Content-Type", "text/plain");
        response.setBody("Error processing the redirection request.");
        response.buildResponse();

        client.sendData(response.getFullResponse());
        return 500;
    }
}

void Server::eraseClient(int event_fd) {
    // Find all clients in _clients where the socket matches event_fd.
    auto it = std::find_if(_clients.begin(), _clients.end(), [&](const Client &c) {
            // Log each comparison for debugging
            // std::cout << "Checking client with socket FD: " << c.getSocket()
            //           << " against target FD: " << event_fd << std::endl;
            
            // Return true if this client should be removed
            return c.getSocket() == event_fd;
        }
    );

    // Check if any clients were marked for removal
    if (it != _clients.end()) {
        // Log removal
        // std::cout << "Removing client with FD: " << event_fd 
        //           << ". Total clients before removal: " << _clients.size() << std::endl;

        // Physically remove the clients from the container
        _clients.erase(it);
		_clients.shrink_to_fit();

        // Log success
        // std::cout << "We are removing this fd: "<< event_fd << " Client removed. Total clients after removal: " << _clients.size() << std::endl;
        // std::cout << "Client capacity after removal: " << _clients.capacity() << std::endl;
		// printClientsVector(_clients);
    } else {
        // Log if no matching client was found
        // std::cerr << "Warning: No client found with FD: " << event_fd << std::endl;
    }
}


void Server::setRedirect(const std::string& statusCode, const std::string& redirectPath) {
        if (statusCode.empty() || redirectPath.empty()) {
        _redirect.first = 0;
        _redirect.second = "";
        return;
    }
       
       for (char c : statusCode) {
        if (!std::isdigit(c)) {
            throw std::invalid_argument("Status code must contain only digits");
        }
   }
    _redirect.first = std::stoi(statusCode);
    _redirect.second = redirectPath;
}