#include "../include/Config.hpp"
#include "../include/Server.hpp"
#include "../include/Location.hpp"
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>

Location::Location() {}
Location::~Location() {}

void Location::set_root(const std::string& root) {
    this->_root = root;
}

void Location::set_autoindex(bool autoindex) {
    this->_autoindex = autoindex;
}

void Location::set_allowed_methods(const std::vector<std::string>& methods) {
    this->_allowed_methods = methods;
}

void Location::set_cgi_pass(const std::string& cgi_pass) {
    _cgi_pass = cgi_pass;
}

void Location::set_cgi_path(const std::string& cgi_path) {
    _cgi_path = cgi_path;
}

void Location::print_info() const {
    std::cout << "    Root: " << _root << std::endl;
    std::cout << "    Autoindex: " << (_autoindex ? "on" : "off") << std::endl;

    std::cout << "    Allowed methods: ";
    for (const std::string& method : _allowed_methods) {
        std::cout << method << " ";
    }
    std::cout << std::endl;

    std::cout << "    CGI pass: " << _cgi_pass << std::endl;
    std::cout << std::endl;
}
