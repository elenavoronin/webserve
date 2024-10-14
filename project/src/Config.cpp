#include "../include/Config.hpp"
#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>


Config::Config() {
    // std::cout << "Config constructor called" << std::endl;
}

Config::~Config() {
    // std::cout << "Config destructor called" << std::endl;
}

std::vector<std::string> tokenize(const std::string &line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}



bool isFileEmpty(const std::string& fileName) {
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    return file.tellg() == 0;
}

std::vector<Server> parse_config(std::ifstream &file) {
    std::vector<Server> servers;
    Server current_server;
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty()) {
            std::vector<std::string> tokens = tokenize(line);
            if (tokens.size() >= 2) {
                // Mapping tokens to Server class attributes
                std::string key = tokens[0];
                std::string value = tokens[1];

                if (key == "server_name") {
                    current_server.set_server_name(value);
                } else if (key == "port") {
                    current_server.set_port(value.c_str());
                } else if (key == "root") {
                    current_server.set_root(value);
                } else if (key == "autoindex") {
                    current_server.set_autoindex("on");
                } else if (key == "cgi_path") {
                    current_server.set_cgi_path(value);
                } else if (key == "upload_store") {
                    current_server.set_upload_store(value);
                } else if (key == "allowed_methods") {
                    // Assuming methods are comma-separated in config file
                    std::istringstream method_stream(value);
                    std::string method;
                    std::vector<std::string> methods;
                    while (std::getline(method_stream, method, ',')) {
                       methods.push_back(method);
                    }
                    current_server.set_allowed_methods(methods);
                } else if (key == "default_file") {
                    current_server.set_default_file(value);
                }
            }
        }
    }
    servers.push_back(current_server);
    return servers;
}

int Config::check_config(const std::string &config_file) {
    std::ifstream file(config_file.c_str());
    if (!file) {
        std::cerr << "Error: Cannot open config file." << std::endl;
        return -1;
    }
    if (isFileEmpty(config_file) == true) {
        std::cerr << "Error: Cannot open config file." << std::endl;
        return -1;
    }
    _servers = parse_config(file);
    file.close();
    return 0;
}


int main() {
    Config config;

    // Assuming you have a config file "server_config.txt"
    config.check_config("server_config.txt");

    // Print server information
    for (const auto& server: config._servers) {
        server.print_info();
        std::cout << "---------------------------" << std::endl;
    }

    return 0;
}