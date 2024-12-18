#include "../include/HttpRequest.hpp"

/**
 * @brief       Default constructor for the HttpRequest class.
 * 
 * @details     Initializes an empty HttpRequest object with no fields or received data.
 */
HttpRequest::HttpRequest(){
	_strReceived = "";
}

HttpRequest::~HttpRequest(){}


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
	std::string method, path, version, host;
	std::string line;
	request_stream >> method >> path >> version; 
	setField("method", method);
	setField("path", path);
	setField("version", version); 
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
	std::string len = this->getField("Content-length");
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