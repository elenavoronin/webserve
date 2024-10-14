#include "./include/HttpRequest.hpp"

HttpRequest::HttpRequest(std::string method, std::string path, std::string version){
	this->method = method;
	this->path = path;
	this->version = version;
}

HttpRequest::~HttpRequest(){}

HttpRequest& HttpRequest::operator=(const HttpRequest& copy){
    if (this != &copy) { // CORRECT?
	this->method = copy.method;
	this->path = copy.path;
	this->version = copy.version;
    }
    return *this;
}

HttpRequest::HttpRequest(const HttpRequest& copy){
	this->method = copy.method; // CORRECT?
	this->path = copy.path;
	this->version = copy.version;
}
