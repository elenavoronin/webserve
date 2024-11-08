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
		void print_config_parse() const {
		std::vector<Server> servers = get_servers();
		for (std::vector<Server>::const_iterator serverIt = servers.begin(); serverIt != servers.end(); ++serverIt) {
			serverIt->print_info();
			std::map<std::string, Location> locations = serverIt->get_locations();
			for (std::map<std::string, Location>::const_iterator locIt = locations.begin(); locIt != locations.end(); ++locIt) {
				std::cout << "Location Path: " << locIt->first << std::endl; // Print the path
				locIt->second.print_info(); // Print information of the location
		}
		}
		std::cout << "---------------------------" << std::endl;
		}
};
