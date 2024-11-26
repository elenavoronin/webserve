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
    const std::string& get_root() const { return _root; }
    const std::vector<std::string>& get_allowed_methods() const { return _allowed_methods; }
    bool  get_autoindex() const { return _autoindex; }
    const std::string& get_cgi_pass() const { return _cgi_pass; }
    const std::string& get_cgi_path() const { return _cgi_path; }
};
