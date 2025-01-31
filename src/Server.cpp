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
	
    // std::cout << "OPEN!!! New connection accepted on fd: " << new_fd << std::endl;

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
        // std::cerr << "Client not found for fd: " << event_fd << std::endl;
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
                this->setUploadStore(defaultServer.getUploadStore()); // Use the upload path from the Server config, not Location?
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
    std::cout << "Method: " << method << std::endl;
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

    if (request->getPath() == "/") {
        filepath = this->getRoot() + '/' + this->getIndex();
        request->setFullPath(filepath);
    }
    
    if (filepath.find("/cgi-bin") != std::string::npos) { 
        request->setFullPath(filepath);
        client.startCgi(request);
        return 0;
    }

    if (!fileExists(filepath)) {
        return sendErrorResponse(client, 404, "www/html/404.html");
    }

    client.prepareFileResponse(readFileContent(filepath));
    client.sendData(client.getHttpResponse()->getFullResponse());
    response.buildResponse();

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
        std::string pathToDelete = request->getPathToDelete(request->getRawRequest());
        if (pathToDelete.empty() || !fileExists(pathToDelete)) {
            return sendErrorResponse(client, 404, "www/html/404.html");
        }

        if (pathToDelete.find("/cgi-bin") != std::string::npos) {
            request->setFullPath(pathToDelete);
            client.startCgi(request);
            return 0;
        }

        if (remove(pathToDelete.c_str()) != 0) {
            throw std::runtime_error("Error deleting file: " + pathToDelete);
        }
        
        response.setStatus(200, getStatusMessage(200));
        response.setHeader("Content-Type", "text/plain");
        response.setBody("Data deleted successfully.");
        response.buildResponse();
        client.sendData(response.getFullResponse());
        
        return 200;
    } catch (const std::exception& e) {
        return handleServerError(client, e, "Error handling DELETE request");
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
 * 
 * @note The boundary in multipart/form-data separates different parts of an HTTP request body, 
 * including text fields and file uploads. 
 * It ensures proper parsing by marking where each part starts and ends. 
 * Without it, the server couldn't distinguish between fields and file content, 
 * leading to incorrect data extraction and processing.
 * ------WebKitFormBoundaryxyz123 is a boundary used in multipart/form-data requests.
 */
int Server::handlePostRequest(Client &client, HttpRequest* request) {
    HttpResponse response;
    try {
        if (request->getPath().find("/cgi-bin") != std::string::npos) {
            request->setFullPath(request->getPath());
            client.startCgi(request);
            return 0;
        }

        std::string uploadPath = getUploadStore(); // Ensure correct upload path
        std::cout << "[DEBUG] Upload Path: " << uploadPath << std::endl;
        if (uploadPath.empty()) {
            throw std::runtime_error("Upload path not set in configuration");
        }
        ensureUploadDirectoryExists(uploadPath);
        std::cout << "[DEBUG] Upload directory exists" << std::endl;
        
        std::string boundary = extractBoundary(request->getHeader("Content-Type"));
        std::cout << "[DEBUG] Extracted boundary: " << boundary << std::endl;
        std::vector<std::string> parts = splitMultipartBody(request->getBody(), boundary);
         std::cout << "[DEBUG] Total parts detected: " << parts.size() << std::endl;
        

        for (const std::string& part : parts) {
            processMultipartPart(part, uploadPath);
        }

        response.setStatus(201, "Created");
        response.setHeader("Content-Type", "text/plain");
        response.setBody("File uploaded successfully.");
        response.buildResponse();
        
        client.sendData(response.getFullResponse());
        std::cout << "[DEBUG] Upload request processed successfully." << std::endl;
        return 201;
    } catch (const std::exception& e) {
        return handleServerError(client, e, "Error handling POST request");
    }
}

/**
 * @brief Ensures that the uploads directory exists.
 * 
 * This method checks if the "uploads" directory exists and if not, it creates it.
 * The directory is used to store uploaded files.
 */
void Server::ensureUploadDirectoryExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {    
        if (mkdir(path.c_str(), 0777) == -1) {
            throw std::runtime_error("Failed to create directory: " + path);
        }
    }
}

/**
 * @brief       Extracts the boundary from a Content-Type header of a multipart/form-data request.
 * 
 * @param       contentType    The Content-Type header value.
 * 
 * @return      The extracted boundary as a string, prefixed with "--".
 * 
 * @throws      std::runtime_error  If the Content-Type is not multipart/form-data,
 *                                  or if the boundary is missing.
 */
std::string Server::extractBoundary(const std::string& contentType) {
    if (contentType.find("multipart/form-data") == std::string::npos) {
        throw std::runtime_error("Unsupported Content-Type: Only multipart/form-data is supported.");
    }

    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos == std::string::npos) {
        throw std::runtime_error("Malformed multipart/form-data request: Missing boundary.");
    }

    return "--" + contentType.substr(boundaryPos + 9); // Extract boundary
}

/**
 * @brief       Splits a multipart/form-data request body into individual parts.
 * 
 * @param       requestBody    The raw request body as a string.
 * @param       boundary       The boundary string as a string, prefixed with "--".
 * 
 * @return      A vector of strings, each containing a part of the request body.
 * 
 * @details     This function splits the request body into individual parts,
 *              using the provided boundary string as a delimiter. Each part
 *              is a string containing the data of that part of the request body.
 *              The parts do not include the boundary string.
 * 
 * @throws      std::runtime_error  If the request body is malformed (e.g. missing
 *                                  boundary, or a part is missing its boundary).
 */
std::vector<std::string> Server::splitMultipartBody(const std::string& requestBody, const std::string& boundary) {
    std::vector<std::string> parts;
    std::stringstream bodyStream(requestBody);
    std::string line, part;

    while (std::getline(bodyStream, line)) {
        if (line.find(boundary) != std::string::npos) {
            if (!part.empty()) {
                parts.push_back(part);
                part.clear();
            }
        } else {
            part += line + "\n";
        }
    }
    return parts;
}

/**
 * @brief       Extracts the filename from a part of a multipart/form-data request body.
 * 
 * @param       headers      A string containing the headers of a part of the request body.
 * 
 * @return      The extracted filename as a string, or an empty string if no filename is found.
 * 
 * @details     This function looks for a "filename=" header in the given string,
 *              and extracts the value of that header. If the header is not found,
 *              or the header is malformed, an empty string is returned.
 */
std::string Server::extractFilename(const std::string& headers) {
    size_t filenamePos = headers.find("filename=");
    if (filenamePos == std::string::npos) {
        return "";  // No filename found
    }

    size_t start = filenamePos + 9; // Move past 'filename="'
    size_t end = headers.find("\"", start);
    if (end == std::string::npos) {
        return ""; // Malformed header
    }

    return headers.substr(start, end - start); // Extract filename
}

/**
 * @brief       Processes a single part of a multipart/form-data request body.
 * 
 * @param       part        The part of the request body as a string.
 * 
 * @details     This function extracts the filename from the given part of the
 *              request body, and saves the file content to the specified path.
 *              If the part contains no filename, the default filename is used.
 *              If the part is malformed, it is skipped.
 */
void Server::processMultipartPart(const std::string& part, const std::string& uploadPath) {
    // Find Content-Disposition header
    size_t headerEnd = part.find("\r\n\r\n");
    if (headerEnd == std::string::npos) return; // Skip malformed parts

    std::string headers = part.substr(0, headerEnd);  // Extract headers

    // Extract filename
    std::string filename = extractFilename(headers);
    if (filename.empty()) {
        filename = "uploaded_file"; // Default filename
    }

    // Save file content
    std::string savePath = uploadPath + filename;
    saveUploadedFile(savePath, part, headerEnd + 4);
}

/**
 * @brief Saves the content of a multipart/form-data part to a file.
 *
 * This function writes the data from a specified starting point in the 
 * multipart/form-data part to a file at the given path. If the file cannot be 
 * opened for writing, an exception is thrown.
 *
 * @param filePath The path to the file where the data should be saved.
 * @param part The multipart/form-data part containing the file content.
 * @param dataStart The starting index within the part from which to begin writing.
 *
 * @throws std::runtime_error If the file cannot be opened for writing.
 */

void Server::saveUploadedFile(const std::string& filePath, const std::string& part, size_t dataStart) {
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filePath);
    }

    outFile.write(part.data() + dataStart, part.size() - dataStart);
    outFile.close();
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
    std::cout << "Handling Redirection..." << std::endl;
    (void)request;

    try {
        response.setStatus(getRedirect().first, getStatusMessage(getRedirect().first));
        response.setHeader("Location", getRedirect().second);
        response.setHeader("Content-Type", "text/html");
        
        std::string body = "<html><body><h1>Redirecting...</h1>"
                           "<p>If you are not redirected automatically, "
                           "<a href=\"" + getRedirect().second + "\">click here</a>.</p></body></html>";
        response.setBody(body);
        response.buildResponse();

        client.sendData(response.getFullResponse());
        std::cout << "Redirected to: " << getRedirect().second << " with status code: " << getRedirect().first << std::endl;

        return getRedirect().first;
    } catch (const std::exception& e) {
        return handleServerError(client, e, "Error handling redirection request");
    }
}

/**
 * @brief Handles an error encountered while handling a client request.
 *
 * This function writes an error message to the standard error stream and
 * sends a 500 Internal Server Error response to the client with a simple
 * HTML page indicating that an internal server error occurred.
 *
 * @param client The client connection to send the error response to
 * @param e The exception that caused the error
 * @param errorMessage A string describing the error
 *
 * @return The status code of the response (500)
 */
int Server::handleServerError(Client &client, const std::exception &e, const std::string &errorMessage) {
    std::cerr << errorMessage << ": " << e.what() << std::endl;
    return sendErrorResponse(client, 500, "www/html/500.html");
}

/**
 * @brief Sends an error response to the client with a given status code and error page.
 *
 * This function sends a response with the given status code and an HTML page
 * containing the given error page path. If the error page path does not exist,
 * a simple HTML page is sent with the status code and a title indicating the
 * error status.
 *
 * @param client The client connection to send the error response to
 * @param statusCode The status code of the error response
 * @param errorPagePath The path to the HTML page to send as the response body
 *
 * @return The status code of the response
 */
int Server::sendErrorResponse(Client &client, int statusCode, const std::string &errorPagePath) {
    HttpResponse response;
    std::string errorContent = readFileContent(errorPagePath);
    if (errorContent.empty()) {
        errorContent = "<html><body><h1>" + std::to_string(statusCode) + " - Error</h1></body></html>";
    }
    
    response.setStatus(statusCode, getStatusMessage(statusCode));
    response.setHeader("Content-Type", "text/html");
    response.setBody(errorContent);
    response.buildResponse();
    
    client.sendData(response.getFullResponse());
    return statusCode;
}

/**
 * @brief Checks if a file exists.
 *
 * This function takes a path to a file and checks if it exists. It returns true
 * if the file exists and false if it does not.
 *
 * @param path The path to the file to check
 *
 * @return true if the file exists, false if it does not
 */
bool Server::fileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

/**
 * @brief Erases a client with the specified event file descriptor from the server's internal list.
 *
 * This function takes an event file descriptor and checks if any clients in the server's internal
 * list of clients have a matching file descriptor. If a client is found, it is removed from the
 * list. This function is used to remove clients from the server's internal list when the client
 * disconnects.
 *
 * @param event_fd The event file descriptor to search for
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

/**
 * @brief Sets the redirect status code and path for this server.
 *
 * This function takes a status code and a redirect path as parameters and sets
 * the server's internal redirect status code and path to these values. If either
 * parameter is empty, the server's internal redirect status code and path are
 * reset to 0 and an empty string respectively.
 *
 * @param statusCode The status code of the redirect response
 * @param redirectPath The path to redirect to
 *
 * @throws std::invalid_argument If the status code contains non-digit characters
 */
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