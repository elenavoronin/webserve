#include "../include/HttpRequest.hpp"

// HttpRequest::HttpRequest(std::string method, std::string path, std::string version){
// 	// this->method = method;
// 	// this->path = path;
// 	// this->version = version;
// }
HttpRequest::HttpRequest(){
	_strReceived = "";
	// std::cout << "HttpRequest() " << _strReceived << std::endl;
}
HttpRequest::~HttpRequest(){}

HttpRequest& HttpRequest::operator=(const HttpRequest& copy){
	// std::cout << "HttpRequest::operator= 1"  << std::endl;

    if (this != &copy) { // CORRECT?
	this->method = copy.method; // CORRECT?
	this->path = copy.path;
	this->version = copy.version;
	this->_strReceived = copy._strReceived;
	this->_request = copy._request;
    }
	// std::cout << "HttpRequest::operator= 2"  << std::endl;
    return *this;
}

HttpRequest::HttpRequest(const HttpRequest& copy){
	// std::cout << "HttpRequest(const HttpRequest& copy) 1"  << std::endl;
	this->method = copy.method; // CORRECT?
	this->path = copy.path;
	this->version = copy.version;
	this->_strReceived = copy._strReceived;
	this->_request = copy._request;
	// std::cout << "HttpRequest(const HttpRequest& copy) 2"  << std::endl;
}

void HttpRequest::setField(std::string key, std::string value){
	_request[key] = value;
}

std::string HttpRequest::trim(std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

std::string HttpRequest::getField(std::string key){
	auto it = _request.find(key);
	if (it != _request.end()){
		return trim(it->second); //added trim
	}
	return "";
}

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
		int colon = line.find(':');
		if (colon != std::string::npos){
			std::string key = line.substr(0, colon);
			key = trim(key);
			std::string value = line.substr(colon + 1);
			value = trim(value);
			// std::cout << "This is the test line " << key << "    " << value << std::endl;
		}
	}
}

int HttpRequest::checkErrors() {
    // std::cout << "Checking for errors. Method: |" << getField("method") << "|"<< " Version: " << getField("version") << std::endl;
    if (getField("method") != "GET" && method != "POST" && method != "DELETE") {
        std::cerr << "Error: Invalid method." << std::endl;
        return 405;
    }
    if (getField("version").empty()) {
        std::cerr << "Error: Version is empty or invalid!" << std::endl;
        return 400;
    }
    if (getField("version") != "HTTP/1.1") { //should we accept HTTP/1.0?
/*
For persistent connections (such as in HTTP/1.1), you would leave the client in the pfds list to handle further requests.
For non-persistent connections (such as in HTTP/1.0), it's appropriate to remove the client after processing the request.
*/
        std::cerr << "Error: Invalid HTTP version." << std::endl;
        return 400;
    }
    return 200;
}