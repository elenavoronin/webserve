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
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
    
    std::string bindAddress = (getServerName() == "localhost") ? "127.0.0.1" : "0.0.0.0";

    if ((status = getaddrinfo(bindAddress.c_str(), getPortStr().c_str(), &hints, &servinfo)) != 0) {
        throw std::runtime_error("Error get Address information");
        return 1;
    }
	for (newConnect = servinfo; newConnect != NULL; newConnect= newConnect->ai_next){
		if ((serverSocket = socket(newConnect->ai_family, newConnect->ai_socktype, newConnect->ai_protocol)) == -1){
            throw std::runtime_error("Socket Error");
            continue;
		}
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

        if (bind(serverSocket, newConnect->ai_addr, newConnect->ai_addrlen) == -1){
			throw std::runtime_error("Bind Error");
            close(serverSocket);
			continue;
		}
		break;
	}

	freeaddrinfo(servinfo);

	if (newConnect == NULL) {
		throw std::runtime_error("Failed to bind to address");
        return -1;
    }
	if (listen(serverSocket, BACKLOG) == -1) {
        close(serverSocket);
        return -1;
    }
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
 */
int Server::reportReady(EventPoll &eventPoll){
	int listener = getListenerSocket();
    if (listener == -1){
        std::cerr << "error on port: " << getPortStr() << std::endl;
		throw std::runtime_error("Error get listener socket");
	}
    eventPoll.addPollFdEventQueue(listener, POLLIN);
    return listener;
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
 */
void Server::handleNewConnection(EventPoll &eventPoll){

	int new_fd = accept(_listener_fd, nullptr, nullptr);
    if (new_fd == -1) {
        return;
    }
	Client newClient(new_fd, eventPoll);
    _clients.push_back(newClient);
    eventPoll.addPollFdEventQueue(new_fd, POLLIN);
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
 */
void Server::handlePollEvent(EventPoll &eventPoll, int i, defaultServer defaultS, std::vector<defaultServer> servers) {
    Client *client = nullptr;
    pollfd &currentPollFd = eventPoll.getPollEventFd()[i];
    int event_fd = currentPollFd.fd;

    for (auto &c : _clients) {
        if (c.getSocket() == event_fd || 
			c.getCgiRead() == event_fd || 
			c.getCgiWrite() == event_fd) {
            client = &c;
            break;
        }
    }

    if (!client) {
		    eraseClient(event_fd);
        return;
    }

	client->setStartTime(std::chrono::system_clock::now());
    if (currentPollFd.revents & POLLIN) {
        try {
            if (event_fd != client->getSocket() && event_fd == client->getCgiRead()) {
                client->readFromCgi();
            } else {
                client->readFromSocket(this, defaultS, servers);
            }
        } 
        catch (const std::runtime_error &e) {
            if (event_fd == client->getCgiRead()) {
                handleCgiError(client, 500);
                return;
            }
            client->closeConnection(eventPoll, currentPollFd.fd);
			eraseClient(event_fd);
			return;
        }
    }

    if (currentPollFd.revents & POLLOUT) {

        try {
            if (event_fd != client->getSocket() && event_fd == client->getCgiWrite()) {
                client->writeToCgi();
            } 
            else {
                if (client->writeToSocket() > 0) {
					client->closeConnection(eventPoll, currentPollFd.fd);
					eraseClient(event_fd);
					return;
				}
            }
        } 
        catch (const std::runtime_error &e) {
            if (event_fd == client->getCgiWrite()) {
                handleCgiError(client, 500);
                return;
            }
            client->closeConnection(eventPoll, currentPollFd.fd);
			eraseClient(event_fd);
			return;
        }
    }

    if (currentPollFd.revents & (POLLHUP | POLLRDHUP)) {
        if (event_fd == client->getCgiRead() || event_fd == client->getCgiWrite()) {
            handleCgiError(client, 500);
            return;
        }
        client->closeConnection(eventPoll, currentPollFd.fd);
		eraseClient(event_fd);
		return;
    }
}
   
/**
 * @brief Handles CGI errors by closing the CGI pipes and sending an error response
 *        to the client.
 *
 * This method is called when an error occurs while communicating with a CGI
 * process. It closes the CGI pipes and sends an appropriate error response
 * to the client.
 *
 * @param event_fd The file descriptor that triggered the error.
 * @param client The Client object that owns the CGI process.
 */
void Server::handleCgiError(Client* client, int statusCode) {
        if (!client || !client->getCGI() || !client->getHttpResponse()) {
        std::cerr << "Error: handleCgiError called on a client with no CGI process." << std::endl;
        return;
    }
        int cgiExitStatus;
        pid_t cgiPid = client->getCGI()->getPid();

        if (cgiPid > 0) {
            kill(cgiPid, SIGTERM);
            waitpid(cgiPid, &cgiExitStatus, WNOHANG);
        }

        client->addToEventPollRemove(client->getCgiRead(), POLLIN);
        client->addToEventPollRemove(client->getCgiRead(), POLLOUT);
        client->addToEventPollRemove(client->getCgiWrite(), POLLOUT);
        client->addToEventPollRemove(client->getCgiWrite(), POLLIN);
        
        sendErrorResponse(*client, statusCode);

		delete(client->getCGI());
		client->setToNullCGI();
}

/**
 * @brief Checks and updates the server configuration based on the HTTP request.
 *
 * This function compares the server name from the HTTP request with the current
 * server's name. If they match, it returns immediately. Otherwise, it iterates
 * over a list of defaultServer objects to find a matching server configuration
 * based on the request's server name. If a match is found, it updates the
 * current server's configuration with the properties of the matching server.
 *
 * @param HttpRequest Pointer to the HttpRequest object containing the request details.
 * @param servers A vector of defaultServer objects representing the available server configurations.
 */

void Server::checkServer(HttpRequest* HttpRequest, std::vector<defaultServer> servers) {
    if (getServerName() == HttpRequest->getServerName())
		  return;
	for (std::vector<defaultServer>::iterator it = servers.begin(); it != servers.end(); ++it) {
        if (it->_serverName == HttpRequest->getServerName()) {
            this->setPortString(it->_portString);
            this->setRoot(it->_root);
            this->setIndex(it->_index);
            this->setAllowedMethods(it->_allowedMethods);
            this->setAutoindex(it->_autoindex);
            this->setMaxBodySize(it->_maxBodySize);
            this->setErrorPages(it->_errorPages);
            this->setRedirect(std::to_string(it->_redirect.first), it->_redirect.second);
            this->setUploadStore(it->_uploadStore);
		}
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
 */
void Server::checkLocations(std::string path, defaultServer defaultServer) {
    for (const auto& location : this->getLocations()) {
        std::string newPath = path.substr(0, location.first.size());
        if (newPath == location.first) {
            if (!location.second.empty()) {
                Location loc = location.second[0];
                if (!loc.getRoot().empty())
                    this->setRoot(loc.getRoot());
                if (!loc.getIndex().empty())
                    this->setIndex(loc.getIndex());
                if (!loc.getAllowedMethods().empty())
                    this->setAllowedMethods(loc.getAllowedMethods());
                if (loc.getAutoindex() == "on")
                    this->setAutoindex("on");
                else
                    this->setAutoindex("off");
                if (loc.getMaxBodySize() != 0)
                    this->setMaxBodySize(loc.getMaxBodySize());
                if (!loc.getErrorPages().empty())
                    this->setErrorPages(loc.getErrorPages());
                if (loc.getRedirect().first != 0)
                    this->setRedirect(std::to_string(loc.getRedirect().first), loc.getRedirect().second);
                if (!loc.getUploadPath().empty())
                    this->setUploadStore(loc.getUploadPath());
                else
                    this->setUploadStore(defaultServer._uploadStore);
                return;
            }
        }
    }
    this->setRoot(defaultServer._root);
    this->setIndex(defaultServer._index);
    this->setPortString(defaultServer._portString);
    this->setAllowedMethods(defaultServer._allowedMethods);
    this->setAutoindex(defaultServer._autoindex);
    this->setMaxBodySize(defaultServer._maxBodySize);
    this->setUploadStore(defaultServer._uploadStore);
    this->setErrorPages(defaultServer._errorPages);
    this->setRedirect(std::to_string((defaultServer._redirect.first)), defaultServer._redirect.second);
    return;
    
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
int Server::processClientRequest(Client &client, const std::string& request, HttpRequest* HttpRequest, defaultServer defaultS, std::vector<defaultServer> servers) {
	HttpRequest->readRequest(request);

	std::string method = HttpRequest->getMethod();
    std::string path = HttpRequest->getPath();
    std::string version = HttpRequest->getVersion();

	checkLocations(path, defaultS);
	checkServer(HttpRequest, servers);
    if (getRedirect().first != 0)
    {
        return handleRedirect(client);
    }
    int status = validateRequest(method, version);
	if (status != 200) {
		return sendErrorResponse(client, status);
	}
	if (method == "GET" && std::find(this->_allowedMethods.begin(), this->_allowedMethods.end(), "GET") != this->_allowedMethods.end())
		return handleGetRequest(client, HttpRequest);
	if (method == "POST" && std::find(this->_allowedMethods.begin(), this->_allowedMethods.end(), "POST") != this->_allowedMethods.end())
		return handlePostRequest(client, HttpRequest);
	if (method == "DELETE" && std::find(this->_allowedMethods.begin(), this->_allowedMethods.end(), "DELETE") != this->_allowedMethods.end())
		return handleDeleteRequest(client, HttpRequest);
    else
		return sendErrorResponse(client, 405);
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
    
    std::string filepath = this->getRoot() + request->getPath();
    request->setPathToCgi(filepath);
    DIR* dir = opendir(filepath.c_str());
    if (dir) {
        closedir(dir);
        if (access(filepath.c_str(), R_OK | X_OK) != 0) {
            return sendErrorResponse(client, 403);
        }

        if (getAutoindex() == "on") {
            std::string htmlContent = generateDirectoryListing(filepath, request->getPath());
            client.getHttpResponse()->setHeader("Content-Type", "text/html");
            client.getHttpResponse()->setHeader("Content-Length", std::to_string(htmlContent.size()));
            client.getHttpResponse()->setBody(htmlContent);
            client.getHttpResponse()->buildResponse();
            client.addToEventPollRemove(client.getSocket(), POLLIN);
            client.addToEventPollQueue(client.getSocket(), POLLOUT);
            return 200;
        }
        else 
        {
            if (filepath.back() != '/')
                request->setFullPath(filepath + "/" + getIndex()) ;
            else
                request->setFullPath(filepath + getIndex());
            if (access(request->getFullPath().c_str(), F_OK) != 0) {
                return sendErrorResponse(client, 404);
            }
            client.getHttpResponse()->setHeader("Content-Type", "text/html");
            client.getHttpResponse()->setHeader("Content-Length", std::to_string(readFileContent(filepath).size()));
            client.getHttpResponse()->setBody(readFileContent(request->getFullPath()));
            client.getHttpResponse()->buildResponse();
            client.addToEventPollRemove(client.getSocket(), POLLIN);
            client.addToEventPollQueue(client.getSocket(), POLLOUT);
            return 200;
        }
    }
    if (filepath == "www/html/")
        filepath = filepath + getIndex();
    if (filepath.find("/cgi-bin") != std::string::npos) {
        request->setFullPath(request->getPath());
        client.startCgi(request);
        return 0;
    }
    if (access(filepath.c_str(), F_OK) != 0) {
        return sendErrorResponse(client, 404);
    }
    if (access(filepath.c_str(), R_OK) != 0) {
        return sendErrorResponse(client, 403);
    }
    client.getHttpResponse()->setHeader("Content-Type", "text/html");
    client.getHttpResponse()->setHeader("Content-Length", std::to_string(readFileContent(filepath).size()));
    client.getHttpResponse()->setBody(readFileContent(filepath));
    client.getHttpResponse()->buildResponse();
    client.addToEventPollRemove(client.getSocket(), POLLIN);
    client.addToEventPollQueue(client.getSocket(), POLLOUT);
    return 200;
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
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        return "";
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
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
		return 405;
	}
    if (method.empty()) {
        return 405;
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
 * @return The HTTP status code indicating the result of the request processing.
 */
int Server::handleDeleteRequest(Client &client, HttpRequest* request) {

    std::string pathToDelete = _root + request->getPath();
    request->setPathToCgi(pathToDelete);

    if (pathToDelete.empty() || !fileExists(pathToDelete)) {
        return sendErrorResponse(client, 404);
    }

    if (pathToDelete.find("/cgi-bin") != std::string::npos) {
        request->setFullPath(pathToDelete);
        client.startCgi(request);
        return 0;
    }

    if (remove(pathToDelete.c_str()) != 0) {
        throw std::runtime_error("Error deleting file: " + pathToDelete);
    }
    
    client.getHttpResponse()->setStatus(200, getStatusMessage(200));
    client.getHttpResponse()->setHeader("Content-Type", "text/html");
    client.getHttpResponse()->setBody("Data deleted successfully.");
    client.getHttpResponse()->buildResponse();
    client.addToEventPollRemove(client.getSocket(), POLLIN);
    client.addToEventPollQueue(client.getSocket(), POLLOUT);
    
    return 200;
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
 * 
 * @note The boundary in multipart/form-data separates different parts of an HTTP request body, 
 * including text fields and file uploads. 
 * It ensures proper parsing by marking where each part starts and ends. 
 * Without it, the server couldn't distinguish between fields and file content, 
 * leading to incorrect data extraction and processing.
 * ------WebKitFormBoundaryxyz123 is a boundary used in multipart/form-data requests.
 */
int Server::handlePostRequest(Client &client, HttpRequest* request) {

    size_t requestSize = request->getBody().size();

    if (requestSize > getMaxBodySize()) {
        return sendErrorResponse(client, 413);
    }

    if (request->getPath().find("/cgi-bin") != std::string::npos) {
        request->setFullPath(request->getPath());
        std::string filepath = this->getRoot() + request->getPath();
        request->setPathToCgi(filepath);
        client.startCgi(request);
        return 0;
    }

    std::string uploadPath = getUploadStore();
    if (uploadPath.empty()) {
        throw std::runtime_error("Upload path not set in configuration");
    }
    ensureUploadDirectoryExists(uploadPath);
    
    processMultipartPart(request->getStrReceived(), uploadPath);

    std::string boundary = extractBoundary(client, request->getHeader("Content-Type"));
	if (boundary.empty())
		return 406;

	std::vector<std::string> parts = splitMultipartBody(request->getBody(), boundary);
	for (const std::string& part : parts) {
		processMultipartPart(part, uploadPath);
	}

    client.getHttpResponse()->setStatus(201, "Created");
    client.getHttpResponse()->setHeader("Content-Type", "text/plain");
    client.getHttpResponse()->setBody("File uploaded successfully.");
    client.getHttpResponse()->buildResponse();
    client.addToEventPollRemove(client.getSocket(), POLLIN);
    client.addToEventPollQueue(client.getSocket(), POLLOUT);
    return 201;
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
std::string Server::extractBoundary(Client &client, const std::string& contentType) {
    if (contentType.find("multipart/form-data") == std::string::npos) {
        sendErrorResponse(client, 406);
		return "";
    }

    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos == std::string::npos) {
        throw std::runtime_error("Malformed multipart/form-data request: Missing boundary.");
    }
    return "--" + contentType.substr(boundaryPos + 9);
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
	size_t startPos = 0, endPos = 0;

	while ((startPos = requestBody.find(boundary, startPos)) != std::string::npos) {
		startPos += boundary.length();
		endPos = requestBody.find(boundary, startPos);
		if (endPos != std::string::npos) {
			std::string part = requestBody.substr(startPos, endPos - startPos);
			size_t partStart = part.find_first_not_of("\r\n");
			size_t partEnd = part.find_last_not_of("\r\n");
			if (partStart != std::string::npos && partEnd != std::string::npos) {
				parts.push_back(part.substr(partStart, partEnd - partStart + 1));
			}
			startPos = endPos;
		}
		else {
			int remaining_length = requestBody.length() - startPos;
			std::string part = requestBody.substr(startPos, remaining_length);
			parts.push_back(part);
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
        return "";
    }

    size_t start = filenamePos + 10;
    size_t end = headers.find("\"", start);
    if (end == std::string::npos) {
        return "";
    }
    return headers.substr(start, end - start);
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
int Server::processMultipartPart(const std::string& part, const std::string& uploadPath) {

    size_t headerEnd = part.find("\r\n\r\n");
    if (headerEnd == std::string::npos) return 0;
    std::string headers = part.substr(0, headerEnd);
    
    std::string filename = extractFilename(part);
    if (filename.empty()) {
		return -1;
    }
    std::string savePath = uploadPath + filename;
    saveUploadedFile(savePath, part, headerEnd + 4);
	return 0;
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
 * @return The status code of the response
 */
int Server::handleRedirect(Client& client) {
    client.getHttpResponse()->setStatus(getRedirect().first, getStatusMessage(getRedirect().first));
    client.getHttpResponse()->setHeader("Location", getRedirect().second);
    client.getHttpResponse()->setHeader("Content-Type", "text/html");
    
    std::string body = "<html><body><h1>Redirecting...</h1>"
                        "<p>If you are not redirected automatically, "
                        "<a href=\"" + getRedirect().second + "\">click here</a>.</p></body></html>";
    client.getHttpResponse()->setBody(body);
    client.getHttpResponse()->buildResponse();
    
    client.addToEventPollRemove(client.getSocket(), POLLIN);
    client.addToEventPollQueue(client.getSocket(), POLLOUT);

    return getRedirect().first;
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
int Server::sendErrorResponse(Client &client, int statusCode) {
    std::string errorPath;
    std::string errorContent;

    if (!client.getHttpResponse()) {
        std::cerr << "[ERROR] Cannot send error response, HTTP response is NULL." << std::endl;
        return -1;
    }

    if (getErrorPage(statusCode) != "")
    {
        errorPath = getErrorPage(statusCode);
        errorContent = readFileContent(errorPath);
    }

    if (errorContent.empty()) {
        errorContent = "<html><body><h1>" + std::to_string(statusCode) + " - Error</h1></body></html>";
    }
    
    client.getHttpResponse()->setStatus(statusCode, getStatusMessage(statusCode));
    client.getHttpResponse()->setHeader("Content-Type", "text/html");
    client.getHttpResponse()->setBody(errorContent);
    client.getHttpResponse()->buildResponse();

    client.addToEventPollRemove(client.getSocket(), POLLIN);
    client.addToEventPollQueue(client.getSocket(), POLLOUT);
    
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
    auto it = std::find_if(_clients.begin(), _clients.end(), [&](const Client &c) {
            return c.getSocket() == event_fd;
        }
    );

    if (it != _clients.end()) {
        _clients.erase(it);
		_clients.shrink_to_fit();
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
