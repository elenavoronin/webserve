#include "../include/HttpRequest.hpp"

// HttpRequest::HttpRequest(std::string method, std::string path, std::string version){
// 	// this->method = method;
// 	// this->path = path;
// 	// this->version = version;
// }
HttpRequest::HttpRequest(){}
HttpRequest::~HttpRequest(){}

HttpRequest& HttpRequest::operator=(const HttpRequest& copy){
    if (this != &copy) { // CORRECT?
	// this->method = copy.method;
	// this->path = copy.path;
	// this->version = copy.version;
    }
    return *this;
}

HttpRequest::HttpRequest(const HttpRequest& copy){
	// this->method = copy.method; // CORRECT?
	// this->path = copy.path;
	// this->version = copy.version;
}

void HttpRequest::setField(std::string key, std::string value){
	_request[key] = value;
}

std::string HttpRequest::getField(std::string key){
	auto it = _request.find(key);
	if (it != _request.end())
		return it->second;
	return "";
}

std::string HttpRequest::trim(std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}

void HttpRequest::readRequest(std::string request){
	std::istringstream request_stream(request);
	std::string method, path, version, host; //?
	std::string line;
	request_stream >> method >> path >> version; 
	setField("method", method);
	setField("path", path);
	setField("version", version);
	while(std::getline(request_stream, line)){
		int colon = line.find(':');
		if (colon != std::string::npos){
			std::string key = line.substr(0, colon);
			key = trim(key);
			std::string value = line.substr(colon + 1);
			value = trim(value);
			//std::cout << "This is the test line " << key << "    " << value << std::endl;
		}
	}
}


int HttpRequest::checkErrors() {
    std::cout << "Checking for errors. Method: " << getField("method")<< " Version: " << getField("version") << std::endl;
    if (method != "GET" && method != "POST" && method != "DELETE") {
        std::cerr << "Error: Invalid method." << std::endl;
        return 405;
    }
    if (version.empty()) {
        std::cerr << "Error: Version is empty or invalid!" << std::endl;
        return 400;
    }
    if (version != "HTTP/1.1") {
        std::cerr << "Error: Invalid HTTP version." << std::endl;
        return 400;
    }

    return 200;
}