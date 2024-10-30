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

