#pragma once

// class Client;

#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>
#include "../include/Location.hpp"

class Server;

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
		void print_config_parse() const;
};
