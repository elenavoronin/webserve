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
		client->closeConnection(eventPoll);
		eraseClient(event_fd);
        return;
    }

    // Handle readable events
    if (currentPollFd.revents & POLLIN) {
		std::cout << "POLLIN" << std::endl;
        try {
            if (event_fd != client->getSocket() && event_fd == client->getCgiRead()) {
                client->readFromCgi();
            } else {
                client->readFromSocket(this, defaultServer);
            }
        } catch (const std::runtime_error &e) {
            std::cerr << "Read error: " << e.what() << std::endl;
            client->closeConnection(eventPoll);
			eraseClient(event_fd);
        }
    }
    // Handle writable events
    if (currentPollFd.revents & POLLOUT) {
		std::cout << "POLLOUT" << std::endl;
        try {
            if (event_fd != client->getSocket() && event_fd == client->getCgiWrite()) {
                client->writeToCgi();
            } else {
                if (client->writeToSocket() > 0) {
					client->closeConnection(eventPoll);
					eraseClient(event_fd);
				}
            }
        } catch (const std::runtime_error &e) {
            std::cerr << "Write error: " << e.what() << std::endl;
            client->closeConnection(eventPoll);
			eraseClient(event_fd);
        }
    }

    // Handle hangup or disconnection events
    if (currentPollFd.revents & (POLLHUP | POLLRDHUP)) {
		// std::cout << "does this happen" << std::endl;
        client->closeConnection(eventPoll);
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
	//std::cout <<" This is request "<< request << std::endl;
	HttpRequest->readRequest(request);

	std::string method = HttpRequest->getMethod();
    std::string path = HttpRequest->getPath();
    std::string version = HttpRequest->getVersion();

	checkLocations(path, defaultServer);
	int status = validateRequest(method, version);
	if (status != 200) {
		sendFileResponse(client.getSocket(), "www/html/500.html", status);  //change to a config ones?
		return status;
	}
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
 * @todo writing needs to go through the poll loop not working yet
 */
int Server::handleGetRequest(Client &client, HttpRequest* request) {
	
	std::string filepath = this->getRoot() + request->getPath();
	request->setFullPath(filepath);

	if (request->getPath() == "/") {
		filepath = this->getRoot() + '/' + this->getIndex();
		request->setFullPath(filepath);
	}
	if (request->getPath().rfind(_index, 0) == 0) { //changed to config
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
        client.prepareFileResponse();
        return 200;

    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        sendFileResponse(client.getSocket(), "www/html/404.html", 404);
        return 404;
    }
	// sendFileResponse(client.getSocket(), filepath, 200);//needs to be set respionse because need to go back to poll loop
	return 200;
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
		sendBody(clientSocket, "<html><body>404 - File Not Found</body></html>");
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

/*
Extract the path from the request (usually the file or resource to be deleted).
Check if the requested resource exists. If not, return a 404 Not Found response.
For security reasons, it's essential to implement some form of authorization to ensure that only authorized users can delete resources.
If the user isn't authorized, return a 401 Unauthorized or 403 Forbidden.
Ensure the resource being deleted is a valid file, directory, or resource that can be deleted. For example, check if it’s a file in a specific directory on the server.
Optionally, check if the file can be deleted (i.e., it’s not locked or in use).
Perform the deletion. For files or directories, use appropriate system calls to delete the resource.
If it’s a database entry or another type of resource, ensure the record is properly deleted from the data store.
Check if there are any issues while deleting (e.g., file permissions, file not found, or resource is locked).
If an error occurs during the deletion process, return a 500 Internal Server Error or a more specific status code.
If the deletion was successful, send a 200 OK or 204 No Content response.
If there was an issue, return a corresponding error code:
403 Forbidden: If the user is not allowed to delete the resource.
404 Not Found: If the file or resource does not exist.
500 Internal Server Error: If there was an error during the deletion process.
It’s often useful to log the deletion operation for auditing purposes, especially if your server manages important data.
As with the POST request, decide whether to close the connection or keep it alive based on the HTTP version or the Connection header.
*/
int Server::handleDeleteRequest(Client &client, HttpRequest* Http) {
	(void)client;
	(void)Http;

	return 0;
}

/*Extract the HTTP headers to understand what type of data is being sent (e.g., Content-Type, Content-Length).
If the body size exceeds a predefined limit (for example, from a config file), return an error like 413 Request Too Large.
Ensure that the Content-Length header is present and valid. This will tell you how much data to expect in the body.
Read the body of the request from the socket. You might need to handle partial reads (i.e., the body could arrive in chunks).
Form Data: If Content-Type is application/x-www-form-urlencoded, parse the form fields and their values.
JSON Data: If the request contains application/json, you can parse the JSON data and extract the necessary information.
File Uploads: If the Content-Type is multipart/form-data, handle file uploads (you'll need to parse the file boundaries and save the file to disk).
Form Submission: Save data to a database, perform an action, or return a response to confirm the form was submitted.
File Upload: Save the file to a specific directory and generate a success/failure response.
Once processing is complete, send an appropriate response to the client. 
Success (200 OK): If the request was processed successfully.
Error (400 Bad Request): If there was a problem with the data.
Error (500 Internal Server Error): If something went wrong on the server side.
Decide whether to close the connection or keep it alive (based on HTTP version or a Connection header).
*/
int Server::handlePostRequest(Client &client, HttpRequest* Http) {

    (void)client;
    (void)Http;
//  	std::string requestBody = Http->getBody();
//     std::string contentType = Http->getHeader("Content-Type");
//     std::string contentLength = Http->getHeader("Content-Length");

//     // Validate content length
//     if (requestBody.empty() || contentLength.empty()) {
//         sendErrorResponse(client.getSocket(), 400, "Bad Request: No body provided");
//         return 400;
//     }

//     // Check if request body size exceeds max allowed size
//     if (requestBody.size() > this->_maxBodySize) {
//         sendErrorResponse(client.getSocket(), 413, "Payload Too Large");
//         return 413;
//     }

//     // Determine if it's a file upload
//     if (contentType.find("multipart/form-data") != std::string::npos) {
//         return handleFileUpload(client, Http);
//     }

//     // Save request body to a file (for example, logs)
//     std::string filePath = "www/uploads/post_data.txt";
//     std::ofstream outFile(filePath, std::ios::app);
//     if (!outFile) {
//         sendErrorResponse(client.getSocket(), 500, "Internal Server Error: Cannot write to file");
//         return 500;
//     }
//     outFile << requestBody << std::endl;
//     outFile.close();

//     // Send success response
//     sendSuccessResponse(client.getSocket(), 201, "Created");
    return 201; // HTTP 201 Created
}


/**
 * @brief Erase a client from the server's list of active clients.
 *
 * This function is used to remove a client from the server's list of active clients.
 * It takes the file descriptor of the socket associated with the client as an argument.
 *
 * @param[in] event_fd The file descriptor of the socket associated with the client to be removed.
 *
 * @return Nothing.
 */
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
