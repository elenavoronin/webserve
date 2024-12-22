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

void printConfigParse(Config &config) {
std::vector<Server> servers = config.getServers();
for (std::vector<Server>::const_iterator serverIt = servers.begin(); serverIt != servers.end(); ++serverIt) {
    printInfo(*serverIt);
    std::map<std::string, std::vector<Location>> locations = serverIt->getLocations();
    for (std::map<std::string, std::vector<Location>>::const_iterator locIt = locations.begin(); locIt != locations.end(); ++locIt) {
        std::cout << "Location Path: " << locIt->first << std::endl; // Print the path
        const std::vector<Location>& locationVector = locIt->second;
            for (std::vector<Location>::const_iterator vecIt = locationVector.begin(); vecIt != locationVector.end(); ++vecIt) {
                vecIt->printInfo(); // Print information about the location
            }
}
}
std::cout << "---------------------------" << std::endl;
}

void printTokens(const std::vector<std::string>& tokens) {
    std::cout << "Tokens: ";
    for (size_t i = 0; i < tokens.size(); ++i) {
        std::cout << tokens[i];
        if (i != tokens.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << std::endl;
}

void printInfo(const Server &server) {
    std::cout << "Server Name: " << server.getServer_name() << std::endl;
    std::cout << "Port: " << server.getPortStr() << std::endl;
    std::cout << "Root: " << server.getRoot() << std::endl;
    std::cout << "Index: " << server.getIndex() << std::endl;

    // Store the result of getAllowed_methods() to avoid dangling references
    const std::vector<std::string>& allowedMethods = server.getAllowed_methods();
    std::cout << "Allowed Methods: ";
    for (std::vector<std::string>::const_iterator it = allowedMethods.begin(); it != allowedMethods.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    // Store the result of getErrorPage() to avoid dangling references
    const std::vector<std::string>& errorPages = server.getErrorPage();
    std::cout << "Error Pages: ";
    for (std::vector<std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}
