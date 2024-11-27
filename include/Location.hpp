#pragma once

// class Server;
// class Client;

#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>



class Location {
  private:
    std::string               _root;
    std::string               _index;
    std::vector<std::string>  _errorPages;
    std::string               _redirect;
    std::vector<std::string>  _allowedMethods;
    bool                      _autoindex;
    std::string               _cgiPass;
    std::string               _cgiPath;
    size_t											_maxBodySize; //TODO to parse in config 

public:
    Location();
    Location(const Location& copy) = default;
    Location& operator=(const Location& copy) = default;
    ~Location();
    void setRoot(const std::string& root);
    void setAllowedMethods(const std::vector<std::string>& methods);
    void setAutoindex(bool autoindex);
    void setCgiPass(const std::string& cgiPass);
    void setCgiPath(const std::string& cgiPath);
    void setIndex(const std::string& index);
    void setRedirect(const std::string& redirect);
    void setErrorPages(const std::vector<std::string>& errorPages);


    void printInfo() const;


    //getters
    const std::string& getRoot() const { return _root; }
    const std::vector<std::string>& getAllowedMethods() const { return _allowedMethods; }
    bool  getAutoindex() const { return _autoindex; }
    const std::string& getCgiPass() const { return _cgiPass; }
    const std::string& getCgiPath() const { return _cgiPath; }
    const std::string& getIndex() const { return _index; }
    const std::string& getRedirect() const { return _redirect; }
    const std::vector<std::string>& getErrorPages() const { return _errorPages; }
};
