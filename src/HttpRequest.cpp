#include "../include/HttpRequest.hpp"

// HttpRequest::HttpRequest(std::string method, std::string path, std::string version){
// 	// this->method = method;
// 	// this->path = path;
// 	// this->version = version;
// }

/**
 * @brief       Default constructor for the HttpRequest class.
 * 
 * @details     Initializes an empty HttpRequest object with no fields or received data.
 */
HttpRequest::HttpRequest(){
	_strReceived = "";
	// std::cout << "HttpRequest() " << _strReceived << std::endl;
}

/**
 * @brief       Destructor for the HttpRequest class.
 * 
 * @details     Ensures proper cleanup of resources used by the HttpRequest object.
 *              Currently, no specific cleanup is needed.
 */
HttpRequest::~HttpRequest(){}

/**
 * @brief       Assignment operator for the HttpRequest class.
 * 
 * @param       copy        The HttpRequest object to copy from.
 * 
 * @return      A reference to the current HttpRequest object.
 * 
 * @details     Performs a deep copy of the _strReceived and _request fields.
 */
HttpRequest& HttpRequest::operator=(const HttpRequest& copy){
	// std::cout << "HttpRequest::operator= 1"  << std::endl;

    if (this != &copy) { // CORRECT?
	this->_strReceived = copy._strReceived;
	this->_request = copy._request;
    }
	// std::cout << "HttpRequest::operator= 2"  << std::endl;
    return *this;
}

/**
 * @brief       Copy constructor for the HttpRequest class.
 * 
 * @param       copy        The HttpRequest object to copy from.
 * 
 * @details     Creates a new HttpRequest object by copying the _strReceived and 
 *              _request fields from the given object.
 */
HttpRequest::HttpRequest(const HttpRequest& copy){
	// std::cout << "HttpRequest(const HttpRequest& copy) 1"  << std::endl;
	this->_strReceived = copy._strReceived;
	this->_request = copy._request;
	// std::cout << "PATH :" << this->_request["path"] << std::endl;
	// std::cout << "HttpRequest(const HttpRequest& copy) 2"  << std::endl;
}

/**
 * @brief       Sets a key-value pair in the internal request field map.
 * 
 * @param       key         The key (e.g., "method", "path").
 * @param       value       The value associated with the key.
 */
void HttpRequest::setField(std::string key, std::string value){
	_request[key] = value;
}

/**
 * @brief       Retrieves the value associated with a specific key in the request map.
 * 
 * @param       key         The key to retrieve the value for.
 * 
 * @return      The value associated with the key, or an empty string if the key does not exist.
 */
std::string HttpRequest::getField(std::string key){
	// std::cout << "KEY IS: " << key << std::endl;
	auto it = _request.find(key);
	if (it != _request.end()){
		// std::cout << "VALUE IS: " << it->second << std::endl;
		return trim(it->second); //added trim
	}
	return "";
}

/**
 * @brief       Retrieves the string that represents the raw received HTTP request.
 * 
 * @return      A reference to the _strReceived string.
 */
std::string& HttpRequest::getStrReceived(){
	return _strReceived;
}

/**
 * @brief       Clears the _strReceived string.
 * 
 * @details     Resets the received request string to an empty state.
 */
void HttpRequest::clearStrReceived(){
	_strReceived = "";
}

/**
 * @brief       Checks if the HTTP request headers have been fully received.
 * 
 * @return      True if the headers have been received; otherwise, false.
 */
bool HttpRequest::isHeaderReceived() const{
	return _headerReceived;
}


/**
 * @brief       Sets the state indicating whether headers have been received.
 * 
 * @param       received    Boolean indicating whether headers are fully received.
 */
void HttpRequest::setHeaderReceived(bool received){
	_headerReceived = received;
}

/**
 * @brief       Parses an HTTP request string and populates the HttpRequest fields.
 * 
 * @param       request     The raw HTTP request string.
 * 
 * @details     Extracts the method, path, version, and headers from the HTTP request 
 *              and stores them in the internal request map.
 */
void HttpRequest::readRequest(std::string request){

	std::istringstream request_stream(request);
	std::string method, path, version, host; //?
	std::string line;
	request_stream >> method >> path >> version; 
	setField("method", method);
	setField("path", path);
	setField("version", version);
	// std::cout << "This is the test line path" << path << std::endl; 
	while(std::getline(request_stream, line)){
		if (line.empty())
			break;
		const unsigned long colon = line.find(':');//changed from int to const unsigned long because flags
		if (colon != std::string::npos){
			std::string key = line.substr(0, colon);
			key = trim(key);
			std::string value = line.substr(colon + 1);
			value = trim(value);
			setField(key, value);
			// std::cout << "key is: " << key << "       value is: " << value << std::endl;
		}
	}
}

/**
 * @brief       Trims leading and trailing whitespace from a string.
 * 
 * @param       str         The string to trim.
 * 
 * @return      A new string with whitespace removed from both ends.
 */
std::string HttpRequest::trim(std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// int HttpRequest::checkErrors() {
//     // std::cout << "Checking for errors. Method: |" << getField("method") << "|"<< " Version: " << getField("version") << std::endl;
//     if (getField("method") != "GET" && method != "POST" && method != "DELETE") {
//         std::cerr << "Error: Invalid method." << std::endl;
//         return 405;
//     }
//     if (getField("version").empty()) {
//         std::cerr << "Error: Version is empty or invalid!" << std::endl;
//         return 400;
//     }
//     if (getField("version") != "HTTP/1.1") { //should we accept HTTP/1.0?
// /*
// For persistent connections (such as in HTTP/1.1), you would leave the client in the pfds list to handle further requests.
// For non-persistent connections (such as in HTTP/1.0), it's appropriate to remove the client after processing the request.
// */
//         std::cerr << "Error: Invalid HTTP version." << std::endl;
//         return 400;
//     }
//     return 200;
// }

/**
 * @brief       Determines the content length from an HTTP request string.
 * 
 * @param       request     The raw HTTP request string.
 * 
 * @return      The value of the Content-Length header, or 0 if not present.
 */
int HttpRequest::findContentLength(std::string request){
	this->readRequest(request);
	std::string len = this->getField("Content-length");
	// std::cout << "Content length is " << len << std::endl;
	if (len != "")
		return(std::stoi(len));
	else
		return 0;
}