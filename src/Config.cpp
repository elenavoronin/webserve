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

// Function to remove comments and trim leading/trailing spaces
std::string remove_comments_and_trim(const std::string& line) {
    std::size_t comment_pos = line.find('#');
    std::string trimmed = (comment_pos != std::string::npos) ? line.substr(0, comment_pos) : line;
    
    // Remove leading and trailing whitespace
    std::size_t first = trimmed.find_first_not_of(" \t");
    std::size_t last = trimmed.find_last_not_of(" \t");
    
    return (first == std::string::npos) ? "" : trimmed.substr(first, last - first + 1);
}

std::vector<std::string> Config::tokenize(const std::string &line) {
    std::vector<std::string> tokens;
    
    std::string clean_line = remove_comments_and_trim(line);
    
    if (clean_line.empty()) {
        return tokens; // Skip empty or comment-only lines
    }

    std::stringstream ss(clean_line);
    std::string token;
    
    while (ss >> token) {
        tokens.push_back(token);
        
        // If it's the start of a block (like 'server {'), treat it as a separate token
        if (token == "{" || token == "}") {
            break;
        }
    }
    
    return tokens;
}

bool Config::isFileEmpty(const std::string& fileName) {
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    return file.tellg() == 0;
}

std::vector<Server> Config::parse_config(std::ifstream &file) {
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
                } else if (key == "cgi_pass") {
                    current_server.set_cgi_pass(value);
                } else if (key == "upload_store") {
                    current_server.set_upload_store(value);
                } else if (key == "allowed_methods") {
                    // Assuming methods are separated by a space in config file
                    std::istringstream method_stream(value);
                    std::string method;
                    std::vector<std::string> methods;
                    while (std::getline(method_stream, method, ' ')) {
                       methods.push_back(method);
                    }
                    current_server.set_allowed_methods(methods);
                } else if (key == "default_file") {
                    current_server.set_default_file(value);
                }
                servers.push_back(current_server);
            }
        }
    }
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

    config.check_config("../configs/default.conf");

    // Print server information
    for (std::vector<Server>::const_iterator it = config.get_servers().begin(); it != config.get_servers().end(); ++it) {
        it->print_info();
        std::cout << "---------------------------" << std::endl;
    }

    return 0;
}