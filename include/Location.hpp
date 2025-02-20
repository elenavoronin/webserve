#pragma once

#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>
#include <utility>



class Location {
  private:
    std::string                           _root;
    std::string                           _index;
    std::string                           _return;
    std::map<int, std::string>            _errorPages;
    std::vector<std::string>              _allowedMethods;
    std::string                           _autoindex;
    std::string                           _cgiPass;
    std::string                           _cgiPath;
    size_t										            _maxBodySize;
    std::string                           _cgiExtension;
    std::pair<int, std::string>						_redirect;
    std::string                           _uploadPath;

public:
    Location();
    Location(const Location& copy) = default;
    Location& operator=(const Location& copy) = default;
    ~Location();
    void                        setRoot(const std::string& root);
    void                        setAllowedMethods(const std::vector<std::string>& methods);
    void                        setAutoindex(std::string autoindex);
    void                        setCgiPass(const std::string& cgiPass);
    void                        setCgiPath(const std::string& cgiPath);
    void                        setCgiExtension(const std::string& cgiExtension);
    void                        setIndex(const std::string& index);
    void                        setRedirect(const std::string& statusCode, const std::string& redirectPath);
    void                        setMaxBodySize(const size_t& _maxBodySize);
    void                        setErrorPage(const std::string &statusCode, const std::string &path) { int code = std::stoi(statusCode); _errorPages[code] = path;}
    void                        setUploadPath(const std::string& uploadPath);
    void                        setErrorPages(const std::map<int, std::string>& errorPages) {_errorPages = errorPages;}
    
    const                       std::string& getRoot() const { return _root; }
    const                       std::vector<std::string>& getAllowedMethods() const { return _allowedMethods; }
    std::string                 getAutoindex() const { return _autoindex; }
    const                       std::string& getCgiPass() const { return _cgiPass; }
    const                       std::string& getCgiPath() const { return _cgiPath; }
    const                       std::string& getCgiExten() const { return _cgiExtension; }
    const                       std::string& getIndex() const { return _index; }
    const                       std::pair<int, std::string>& getRedirect() const { return _redirect; }
    const                       std::string& getReturn() const { return _return; }
    const                       size_t& getMaxBodySize() const { return _maxBodySize; }
    const                       std::string getErrorPages(int statusCode) const {try {return _errorPages.at(statusCode); } catch (const std::out_of_range&) {return "";}}
    const                       std::string& getUploadPath() const { return _uploadPath; }
    void                        clearLocation();
    std::map<int, std::string>  getErrorPages() const {return this->_errorPages;}

};
