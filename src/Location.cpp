#include "../include/Config.hpp"
#include "../include/Server.hpp"
#include "../include/Location.hpp"
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>

Location::Location() : _root(), _index(),  _errorPages(), 
      _allowedMethods(), _autoindex(), _cgiPass(), _cgiPath(),
      _maxBodySize(0), _cgiExtension(), _redirect(){}
Location::~Location() {}


void Location::setRoot(const std::string& root) {
    this->_root = root;
}

void Location::setAutoindex(std::string autoindex) {
    this->_autoindex = autoindex;
}

void Location::setAllowedMethods(const std::vector<std::string>& methods) {
    this->_allowedMethods = methods;
}

void Location::setCgiPass(const std::string& cgiPass) {
    _cgiPass = cgiPass;
}

void Location::setCgiExtension(const std::string& cgiExtension) {
    _cgiExtension = cgiExtension;
}

void Location::setCgiPath(const std::string& cgiPath) {
    _cgiPath = cgiPath;
}

void Location::setIndex(const std::string& index) {
    _index = index;
}


void Location::setMaxBodySize(const size_t& maxBodySize) {
    _maxBodySize = maxBodySize;
}

void Location::setRedirect(const std::string& statusCode, const std::string& redirectPath) {
    for (char c : statusCode) {
        if (!std::isdigit(c)) {
            throw std::invalid_argument("Status code must contain only digits");
        }
   }
    _redirect.first = std::stoi(statusCode);
    _redirect.second = redirectPath;
}

void Location::setUploadPath(const std::string& uploadPath) {
    _uploadPath = uploadPath;
}

void Location::clearLocation() {
    _root.clear();                 // Clear the root path
    _index.clear();                // Clear the index
    _errorPages.clear();           // Clear the error pages vector 
    _allowedMethods.clear();       // Clear the allowed methods vector
    _autoindex.clear();           // Reset autoindex to default (false)
    _cgiPass.clear();              // Clear the CGI pass path
    _cgiPath.clear();              // Clear the CGI path
    _maxBodySize = 0;              // Reset max body size to default (0)
    _cgiExtension.clear();         // Clear the CGI extension
    _redirect = std::make_pair(0, ""); // Reset the redirect pair
    _uploadPath.clear(); // Clear the upload path
}