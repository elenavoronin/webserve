#pragma once

// class Server;
// class Client;

#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>



class Location {
  private:
    std::string _root;
    std::vector<std::string> _allowed_methods;
    bool _autoindex;
    std::string _cgi_pass;
    std::string _cgi_path;

public:
    Location();
    Location(const Location& copy) = default;
    Location& operator=(const Location& copy) = default;
    ~Location();
    void set_root(const std::string& root);
    void set_allowed_methods(const std::vector<std::string>& methods);
    void set_autoindex(bool autoindex);
    void set_cgi_pass(const std::string& cgi_pass);
    void set_cgi_path(const std::string& cgi_path);
    void print_info() const;

    //getters
    void get_root(std::string& root) const { root = _root; }
    void get_allowed_methods(std::vector<std::string>& methods) const { methods = _allowed_methods; }
    void get_autoindex(bool& autoindex) const { autoindex = _autoindex; }
    void get_cgi_pass(std::string& cgi_pass) const { cgi_pass = _cgi_pass; }
    void get_cgi_path(std::string& cgi_path) const { cgi_path = _cgi_path; }
};
