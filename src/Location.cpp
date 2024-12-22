#include "../include/Config.hpp"
#include "../include/Server.hpp"
#include "../include/Location.hpp"
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>

Location::Location() : _root(), _index(), _return(), _errorPages(), _redirect(),
      _allowedMethods(), _autoindex(false), _cgiPass(), _cgiPath(),
      _maxBodySize(0), _cgiExtension(){}
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

void Location::setCgiExtension(const std::string& cgiExtension) {
    _cgiExtension = cgiExtension;
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

void Location::setMaxBodySize(const size_t& maxBodySize) {
    _maxBodySize = maxBodySize;
}

void Location::setReturn(const std::string& returns) {
    _return = returns;
}
