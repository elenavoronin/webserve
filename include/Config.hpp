#pragma once

class Server;
// class Client;

#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>

class Config {
    private:
        std::vector<Server>    _servers;

    public:
        Config();
        ~Config();
        int check_config(const std::string &config_file);
		std::vector<std::string> tokenize(const std::string &line);
		bool isFileEmpty(const std::string& fileName);
		std::vector<Server> parse_config(std::ifstream &file);
		void print_servers() const;
        const std::vector<Server>& get_servers() const {
            return _servers;
        }

};


// class Location {
//   private:
//     std::string _root;
//     std::vector<std::string> _allowed_methods;
//     bool _autoindex;
//     std::string _cgi_pass;

// public:
//     Location();
//     ~Location();
//     void set_root(const std::string& root) { _root = root; }
//     void set_allowed_methods(const std::vector<std::string>& methods) { _allowed_methods = methods; }
//     void set_autoindex(bool autoindex) { _autoindex = autoindex; }
//     void set_cgi_pass(const std::string& cgi_pass) { _cgi_pass = cgi_pass; }

// };