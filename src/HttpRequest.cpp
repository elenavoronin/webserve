#include "../include/HttpRequest.hpp"

/**
 * @brief       Default constructor for the HttpRequest class.
 * 
 * @details     Initializes an empty HttpRequest object with no fields or received data.
 */
HttpRequest::HttpRequest() : _headers() {
	_strReceived = "";
}

HttpRequest::~HttpRequest(){}


/**
 * @brief       Sets the path field of the HttpRequest object.
 * 
 * @param       path    The value to assign to the path field.
 * 
 * @return      The assigned path value.
 * @todo		Do i need to use root instead of hardcoded is this correct?
 */
void	HttpRequest::setPath(std::string path) {
	_path = path;
}

/**
 * @brief       Sets the path field of the HttpRequest object.
 * 
 * @param       path    The value to assign to the path field.
 * 
 * @return      The assigned path value.
 * @todo		Do i need to use root instead of hardcoded is this correct? get it from location?
 */
void	HttpRequest::setFullPath(std::string path) {
	_fullPath = path;
}


/**
 * @brief       Sets the method field of the HttpRequest object.
 * 
 * @param       method  The value to assign to the method field.
 * 
 * @return      The assigned method value.
 */
void	HttpRequest::setMethod(std::string method) {
	_method	= method;
}


/**
 * @brief       Sets the version field of the HttpRequest object.
 * 
 * @param       version  The value to assign to the version field.
 * 
 * @return      The assigned version value.
 */

/**
 * @brief       Sets the version field of the HttpRequest object.
 * 
 * @param       version  The value to assign to the version field.
 * 
 * @return      The assigned version value.
 */
void	HttpRequest::setVersion(std::string version) {
	_version = version;
}

/**
 * @brief       Sets the body field of the HttpRequest object.
 * 
 * @param       body    The value to assign to the body field.
 * 
 * @return      The assigned body value.
 */
void HttpRequest::setBody(std::string& body) {
	_body = body;
}

/**
 * @brief       Returns the path of the HTTP request.
 * 
 * @return      The requested path.
 */
std::string	HttpRequest::getPath() {
	return _path;
}

/**
 * @brief       Returns the full path of the HTTP request.
 * 
 * @return      The requested full path.
 */
std::string	HttpRequest::getFullPath() {
	return _fullPath;
}

/**
 * @brief       Returns the HTTP method of the request.
 * 
 * @return      The HTTP method as a string.
 */

std::string	HttpRequest::getMethod() {
	return _method;
}

/**
 * @brief       Returns the HTTP version of the request.
 * 
 * @return      The HTTP version as a string (e.g., "HTTP/1.1").
 */
std::string	HttpRequest::getVersion() {
	return _version;
}


/**
 * @brief       Retrieves the body of the HTTP request.
 * 
 * @return      The body of the request as a string.
 */

std::string HttpRequest::getBody() {
	return _body;
}

/**
 * @brief       Retrieves the value associated with a specific header name.
 * 
 * @param       key         The header name to look up.
 * 
 * @return      The value of the header if it exists, or an empty string if it does not.
 */
std::string HttpRequest::getHeader(const std::string& key) {
	auto it = _headers.find(key);
    if (it != _headers.end()) {
        return it->second; // Return the value if the header exists
    }
    return ""; // Return an empty string if the header is not found
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
	auto it = _request.find(key);
	if (it != _request.end()){
		return trim(it->second);
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
 * @brief       Retrieves the raw received HTTP request as a string.
 * 
 * @details     This method returns the exact string received from the client, without any parsing or processing.
 * 
 * @return      The raw received HTTP request as a string.
 */
std::string HttpRequest::getRawRequest(){
	return _strReceived;
}



std::string HttpRequest::getPathToDelete(const std::string& rawRequest) {
	
	readRequest(rawRequest);
	if (_path.empty()) {
		throw std::runtime_error("Path to resource not found");
	}

	return _path;
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
void HttpRequest::setBodyReceived(bool received){
	_bodyReceived = received;
}
bool HttpRequest::getBodyReceived(){
	return _bodyReceived;
}
/**
 * @brief       Parses an HTTP request string and populates the HttpRequest fields.
 * 
 * @param       request     The raw HTTP request string.
 * 
 * @details     Extracts the method, path, version, and headers from the HTTP request 
 *              and stores them in the internal request map.
 * @todo		Do i need the map or the private variables???
 */
void HttpRequest::readRequest(std::string request){

	std::istringstream request_stream(request);
	std::string line;
	request_stream >> _method >> _path >> _version; 
	setField("method", _method);
	setField("path", _path);
	setField("version", _version);
	while(std::getline(request_stream, line)){
		if (line.empty())
			break;
		const unsigned long colon = line.find(':');
		if (colon != std::string::npos){
			std::string key = line.substr(0, colon);
			key = trim(key);
			std::string value = line.substr(colon + 1);
			value = trim(value);
			setField(key, value);
		}
	}
}

std::string HttpRequest::getServerName() {
	std::string str = getField("Host");
	str.erase(std::remove_if(str.begin(), str.end(),
        [](char c) { return !std::isalpha(c); }), str.end());
	// std::cout << "Server name: " << str << std::endl;
	return str;
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

/**
 * @brief       Determines the content length from an HTTP request string.
 * 
 * @param       request     The raw HTTP request string.
 * 
 * @return      The value of the Content-Length header, or 0 if not present.
 */
int HttpRequest::findContentLength(std::string request){
	this->readRequest(request);
	std::string len = this->getField("Content-Length");
	if (len != "")
		return(std::stoi(len));
	else
		return 0;
}

/**
 * @brief       Sets the string representing the raw received HTTP request.
 * 
 * @param       input       The string to set as the raw received HTTP request.
 * 
 * @details     Replaces the current value of _strReceived with the input string.
 */
void HttpRequest::setStrReceived(std::string input) {
	_strReceived = input;
}
void HttpRequest::setHeader(std::string input) {
	_header = input;
}
std::string HttpRequest::getRequestHeader() {
	return _header;
}

void HttpRequest::parseBody(const std::string& rawRequest) {
	// std::cout << "This Is RawRequest in Parse Body\n " <<  rawRequest << "\n *****"<< std:: endl;
    // Find the double CRLF that separates headers and body
    size_t headerEnd = rawRequest.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        throw std::runtime_error("Invalid HTTP request: Missing headers and body separator");
    }

    // Extract the body (if any)
    size_t bodyStart = headerEnd + 4;  // Skip over "\r\n\r\n"
    if (bodyStart < rawRequest.size()) {
        _body = rawRequest.substr(bodyStart);
    } else {
        _body.clear();  // No body
    }
	setBody(_body);
	// std::cout << "This Is Body in Parse Body " <<  this->_body << "\n *****" << std:: endl;
}

void HttpRequest::parseHeaders(const std::string& rawRequest) {
    size_t headerEnd = rawRequest.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        throw std::runtime_error("Invalid HTTP request: Missing headers");
    }

    std::istringstream headerStream(rawRequest.substr(0, headerEnd));
    std::string line;
    while (std::getline(headerStream, line) && !line.empty()) {
        size_t delimiter = line.find(":");
        if (delimiter != std::string::npos) {
            std::string key = line.substr(0, delimiter);
            std::string value = line.substr(delimiter + 1);
            _headers[key] = value;
			if (key == "Referer value:")
				setPath(value);
        }
    }
	setHeader(rawRequest.substr(0, headerEnd));
	//std::cout << "ONLY HEADER " <<  rawRequest.substr(0, headerEnd) << std::endl;
}
