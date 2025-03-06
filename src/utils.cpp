#include "../include/utils.hpp"

/**
 * @brief Returns the HTTP status message associated with the given status code.
 *
 * Given a HTTP status code, this function returns the corresponding status message.
 * If the status code is not found, the function returns the string "Unknown status".
 *
 * @param statusCode The HTTP status code.
 * @return The corresponding HTTP status message.
 */
std::string getStatusMessage(int statusCode){ //Do I need to add more??????
	std::map<int, std::string> message = {
		{200, "OK"},
		{204, "No Content"},//The request has been successfully processed, but is not returning any content
		{301, "Moved Permanently"},//The requested resource has been assigned a new permanent URI and any future references to this resource SHOULD use one of the returned URIs
		{302, "Found"},//The requested resource resides temporarily under a different URI. Since the redirection might be altered on occasion, the client SHOULD continue to use the Request-URI for future requests
		{400, "Bad Request"}, //The request cannot be fulfilled due to bad syntax
		{401, "Unauthorized"}, //The request was a legal request, but the server is refusing to respond to it. For use when authentication is possible but has failed or not yet been provided
		{404, "Not found"},//The requested page could not be found but may be available again in the future
		{405, "Method Not Allowed"},//A request was made of a page using a request method not supported by that page
		{408, "Request Timeout"}, //The server timed out waiting for the request 
		{413, "Request Too Large"},//The server will not accept the request, because the request entity is too large 
		{500, "Internal Server Error"}, //A generic error message, given when no more specific message is suitable
		{502, "Bad Gateway"},//The server was acting as a gateway or proxy and received an invalid response from the upstream server
		{504, "Gateway Timeout"},//The server was acting as a gateway or proxy and did not receive a timely response from the upstream server
	};
	if (message.count(statusCode))
		return message[statusCode];
	return "Unknown status";
}

/**
 * @brief Prints the configuration of a given server.
 *
 * This function takes a Config object as parameter and prints the configuration
 * of all the servers contained in it. For each server, it prints the server's
 * information and the information of all the locations contained in it.
 *
 * @param config The configuration to be printed.
 */
void printConfigParse(std::vector<Server> &servers) {
for (std::vector<Server>::const_iterator serverIt = servers.begin(); serverIt != servers.end(); ++serverIt) {
    printInfoServer(*serverIt);
    std::map<std::string, std::vector<Location>> locations = serverIt->getLocations();
    for (std::map<std::string, std::vector<Location>>::const_iterator locIt = locations.begin(); locIt != locations.end(); ++locIt) {
        const std::vector<Location>& locationVector = locIt->second;
            for (std::vector<Location>::const_iterator vecIt = locationVector.begin(); vecIt != locationVector.end(); ++vecIt) {
                printInfoLocations(*vecIt);
            }
}
}
std::cout << "---------------------------" << std::endl;
}

/**
 * @brief Prints the contents of a vector of strings to the console.
 *
 * This function is used for debugging purposes to print out the contents of a vector of strings.
 * It takes a const reference to a vector of strings and prints each string with a comma space
 * separator. The last string is not followed by a comma space separator.
 */
void printTokens(const std::vector<std::string>& tokens) {
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i != tokens.size() - 1) {
        }
    }
}

/**
 * @brief Prints detailed information about a server configuration.
 *
 * This function outputs various server properties to the console, including:
 * - Server name
 * - Port
 * - Root path
 * - Index file
 * - Allowed HTTP methods
 * - Error pages
 *
 * It retrieves these details using the appropriate getter methods of the Server class 
 * and iterates over lists to print multiple allowed methods and error pages.
 *
 * @param server A reference to a Server object containing the configuration details.
 */

void printInfoServer(const Server &server) {
    std::cout << "Server Name: " << server.getServerName() << std::endl;
    std::cout << "Port: " << server.getPortStr() << std::endl;
    std::cout << "Root: " << server.getRoot() << std::endl;
    std::cout << "MaxBodySize: " << server.getMaxBodySize() << std::endl;
    std::cout << "Index: " << server.getIndex() << std::endl;
    std::cout << "Autoindex: " << server.getAutoindex() << std::endl;


    // Store the result of getAllowedMethods() to avoid dangling references
    const std::vector<std::string>& allowedMethods = server.getAllowedMethods();
    std::cout << "Allowed Methods: ";
    for (std::vector<std::string>::const_iterator it = allowedMethods.begin(); it != allowedMethods.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    // Retrieve the error pages map
    const std::map<int, std::string>& errorPages = server.getErrorPages();

    std::cout << "Error Pages:" << std::endl;
    for (std::map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it) {
        std::cout << "Status Code: " << it->first << " -> Page: " << it->second << std::endl;
    }

    std::cout << "---------------------------" << std::endl;
}

/**
 * Prints information about a Location object to the standard output
 * @param[in] location The Location object to print information about
 */
void printInfoLocations(const Location &location) {
    std::cout << "    Root: " << location.getRoot() << std::endl;
    std::cout << "    Index: " << location.getIndex() << std::endl;
    std::cout << "    MaxBodySize: " << location.getMaxBodySize() << std::endl;
    std::cout << "    Autoindex: " << location.getAutoindex() << std::endl;

    // Store the result of getAllowedMethods() to avoid dangling references
    const std::vector<std::string>& allowedMethods = location.getAllowedMethods();
    std::cout << "    Allowed methods: ";
    for (std::vector<std::string>::const_iterator it = allowedMethods.begin(); it != allowedMethods.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    // Retrieve the error pages map
    const std::map<int, std::string>& errorPages = location.getErrorPages();

    std::cout << "Error Pages:" << std::endl;
    for (std::map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it) {
        std::cout << "Status Code: " << it->first << " -> Page: " << it->second << std::endl;
    }

    std::cout << "    Redirect to : "  << location.getRedirect().first << " " << location.getRedirect().second << std::endl;
    std::cout << std::endl;
    std::cout << "---------------------------" << std::endl;
}

/**
 * @brief Checks if a Location object is empty.
 *
 * This function takes a Location object as parameter and checks if it is empty.
 * A Location object is considered empty if all of its configuration options are
 * set to their default values.
 *
 * @param location The Location object to check.
 *
 * @return true if the Location object is empty, false otherwise.
 */
bool isEmpty(const Location& location) {
    return location.getRoot().empty() && location.getIndex().empty() 
        && location.getAllowedMethods().empty() && location.getCgiPass().empty() 
        && location.getCgiPath().empty() && location.getMaxBodySize() == 0;
}

/**
 * @brief Prints the socket information of each client in the vector.
 *
 * This function iterates over a vector of Client objects and prints the socket
 * associated with each client to the standard output. It is useful for debugging
 * purposes to verify the active client connections.
 *
 * @param Clients A vector of Client objects whose sockets are to be printed.
 */

void printClientsVector(const std::vector<Client>& Clients) {
    for (unsigned long i = 0; i < Clients.size(); i++) {
        std::cout << "client socket: " << Clients[i].getSocket() << std::endl;
    }
    std::cout << std::endl;

}

/**
 * @brief Prints the contents of the EventPoll object.
 *
 * This function takes an EventPoll object as parameter and prints the contents
 * of the poll event queue. It iterates over the vector of pollfd structures
 * and prints the file descriptor of each event.
 *
 * @param eventPoll The EventPoll object to print.
 */
void printEventPoll(EventPoll& eventPoll) {
    for (unsigned long i = 0; i < eventPoll.getPollEventFd().size(); i++) {
        std::cout << "event:  " << eventPoll.getPollEventFd()[i].fd << std::endl;
    }
    std::cout << std::endl;

}

/**
 * @brief Generates an HTML directory listing for the given directory path and request path.
 *
 * Given a directory path and a request path, this function generates an HTML
 * directory listing that includes links to all files and directories in the
 * given directory. The HTML response will include a title with the request path,
 * followed by an unordered list of files and directories. Directory names will
 * have a trailing slash to indicate they are directories.
 *
 * @param[in] directoryPath The path to the directory to list.
 * @param[in] requestPath The path used in the request URL to access the directory.
 * @return An HTML string containing the directory listing.
 */
std::string generateDirectoryListing(const std::string &directoryPath, const std::string &requestPath) {
    std::ostringstream html;
    html << "<html><head><title>Directory Listing</title></head><body>";
    html << "<h1>Index of " << requestPath << "</h1><ul>";

    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;
    if ((dir = opendir(directoryPath.c_str())) == NULL) {
        throw std::runtime_error("Failed to open directory: " + directoryPath);
        return "<html><body><h1>403 Forbidden</h1><p>Directory listing not allowed.</p></body></html>";
    }

    while ((entry = readdir(dir)) != NULL) {
        std::string fileName = entry->d_name;
        std::string fullPath = directoryPath + "/" + fileName;

        // Skip "." and ".."
        if (fileName == "." || fileName == "..") {
            continue;
        }

        // Add file or directory to the HTML response
        html << "<li><a href=\"" << requestPath;
        if (requestPath.back() != '/') {
            html << "/";
        }
        html << fileName;
        // Check if entry is a directory
        if (stat(fullPath.c_str(), &fileStat) == 0 && S_ISDIR(fileStat.st_mode)) {
            fileName += "/";  // Add trailing slash to indicate it's a directory
        }
        html << "\">" << fileName << "</a></li>";
    }
    closedir(dir);

    html << "</ul></body></html>";
    return html.str();
}
