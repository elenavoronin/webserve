#include "../include/Config.hpp"
#include "../include/Server.hpp"
#include "../include/Location.hpp"
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>

Location::Location() {}
Location::~Location() {}

void Location::setRoot(const std::string& root) {
    this->_root = root;
}

void Location::setAutoindex(bool autoindex) {
    this->_autoindex = autoindex;
}

void Location::setAllowedMethods(const std::vector<std::string>& methods) {
    this->_allowedMethods = methods;
}

void Location::setCgiPass(const std::string& cgiPass) {
    _cgiPass = cgiPass;
}

void Location::setCgiPath(const std::string& cgiPath) {
    _cgiPath = cgiPath;
}

void Location::setIndex(const std::string& index) {
    _index = index;
}

void Location::setRedirect(const std::string& redirect) {
    _redirect = redirect;
}

void Location::setErrorPages(const std::vector<std::string>& errorPages) {
    _errorPages = errorPages;
}


void Location::printInfo() const {
    std::cout << "    Root: " << _root << std::endl;
    std::cout << "    Autoindex: " << (_autoindex ? "on" : "off") << std::endl;

    std::cout << "    Allowed methods: ";
	for (std::vector<std::string>::const_iterator it = _allowedMethods.begin(); it != _allowedMethods.end(); ++it) {
			std::cout << *it << " ";
		}
    std::cout << std::endl;

    std::cout << "    CGI pass: " << _cgiPass << std::endl;
    std::cout << std::endl;
}


