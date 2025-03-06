#include "../include/Config.hpp"
#include "../include/Server.hpp"
#include "../include/Location.hpp"
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>

/**
 * @brief Constructs a Location object with default values.
 *
 * The default values are as follows:
 *   - _root: an empty string
 *   - _index: an empty string
 *   - _errorPages: an empty map
 *   - _allowedMethods: an empty vector
 *   - _autoindex: an empty string
 *   - _cgiPass: an empty string
 *   - _cgiPath: an empty string
 *   - _maxBodySize: 0
 *   - _cgiExtension: an empty string
 *   - _redirect: a pair with both elements set to 0
 *
 * @see Location::~Location()
 */
Location::Location() : _root(), _index(),  _errorPages(), 
      _allowedMethods(), _autoindex(), _cgiPass(), _cgiPath(),
      _maxBodySize(0), _cgiExtension(), _redirect(){}
Location::~Location() {}

/**
 * @brief Sets the root path of the location.
 *
 * @param root The new root path for the location.
 */
void Location::setRoot(const std::string& root) {
    this->_root = root;
}

/**
 * @brief Sets the autoindex option for the location.
 *
 * The autoindex option can be "on" or "off". If it is "on", the server will
 * list the contents of the location's root directory in the response body
 * when the client requests the location's root path. If it is "off", the server
 * will instead return a 403 Forbidden response.
 *
 * @param autoindex The new value for the autoindex option.
 */
void Location::setAutoindex(std::string autoindex) {
    this->_autoindex = autoindex;
}

/**
 * @brief Sets the allowed HTTP methods for the location.
 *
 * The allowed methods should be the HTTP methods that the server is allowed
 * to process for this location. If the client sends a request with a method
 * that is not in the allowed methods vector, the server will respond with a
 * 405 Method Not Allowed status code.
 *
 * @param methods The new allowed HTTP methods for the location.
 */
void Location::setAllowedMethods(const std::vector<std::string>& methods) {
    this->_allowedMethods = methods;
}

/**
 * @brief Sets the CGI pass for the location.
 *
 * The CGI pass is the path to the CGI script that will be executed when the
 * client requests a URL that matches the location's root path and the CGI
 * extension. The server will execute the CGI script and pass the request body
 * to it as standard input.
 *
 * @param cgiPass The path to the CGI script.
 */
void Location::setCgiPass(const std::string& cgiPass) {
    _cgiPass = cgiPass;
}

/**
 * @brief Sets the CGI extension for the location.
 *
 * This extension is used to identify files that should be processed
 * with the CGI handler. The server will execute the CGI script when
 * a requested URL matches the location's root path and ends with this
 * extension.
 *
 * @param cgiExtension The CGI extension to set.
 */

void Location::setCgiExtension(const std::string& cgiExtension) {
    _cgiExtension = cgiExtension;
}

/**
 * @brief Sets the path to the CGI script for this location.
 *
 * The CGI script at this path will be executed by the server when the client
 * requests a URL that matches the location's root path and the CGI extension.
 * The server will execute the CGI script and pass the request body to it as
 * standard input.
 *
 * @param cgiPath The path to the CGI script.
 */
void Location::setCgiPath(const std::string& cgiPath) {
    _cgiPath = cgiPath;
}

/**
 * @brief Sets the index file for the location.
 *
 * This function sets the index file for the location, which is the default
 * file that the server will serve when the client requests the location's
 * root path without specifying a specific file.
 *
 * @param index The name of the index file.
 */
void Location::setIndex(const std::string& index) {
    _index = index;
}

/**
 * @brief Sets the maximum body size for the location.
 *
 * This function sets the maximum size of the request body that the server
 * will accept for this location. If a request exceeds this size, the server
 * will respond with an appropriate error.
 *
 * @param maxBodySize The maximum allowed size for the request body in bytes.
 */

void Location::setMaxBodySize(const size_t& maxBodySize) {
    _maxBodySize = maxBodySize;
}

/**
 * @brief Sets the redirect status code and path for this location.
 *
 * This function takes a status code and a redirect path as parameters and sets
 * the server's internal redirect status code and path to these values. If either
 * parameter is empty, the server's internal redirect status code and path are
 * reset to 0 and an empty string respectively.
 *
 * @param statusCode The status code of the redirect response
 * @param redirectPath The path to redirect to
 *
 * @throws std::invalid_argument If the status code contains non-digit characters
 */
void Location::setRedirect(const std::string& statusCode, const std::string& redirectPath) {
    for (char c : statusCode) {
        if (!std::isdigit(c)) {
            throw std::invalid_argument("Status code must contain only digits");
        }
   }
    _redirect.first = std::stoi(statusCode);
    _redirect.second = redirectPath;
}

/**
 * @brief Sets the upload path for the location.
 *
 * This function assigns the specified upload path to the location. The upload
 * path is used to store files uploaded to this location.
 *
 * @param uploadPath The path where uploaded files should be stored.
 */

void Location::setUploadPath(const std::string& uploadPath) {
    _uploadPath = uploadPath;
}

/**
 * @brief Resets all of the location's configuration options to their default values.
 *
 * This function is used to clear out any configuration options that have been
 * set for the location. It is typically used when parsing a configuration file
 * and the location's configuration needs to be reset.
 */
void Location::clearLocation() {
    _root.clear();
    _index.clear();   
    _errorPages.clear();
    _allowedMethods.clear();
    _autoindex.clear();
    _cgiPass.clear();
    _cgiPath.clear();
    _maxBodySize = 0;
    _cgiExtension.clear();
    _redirect = std::make_pair(0, "");
    _uploadPath.clear();
}