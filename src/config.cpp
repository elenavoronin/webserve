#include "../include/config.hpp"
#include "../include/Server.hpp"
#include <fstream>
#include <vector>

std::vector<std::string> tokenize(const std::string &line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

Config parse_config(const std::string &config_file) {
    std::ifstream file(config_file.c_str());
    if (!file) {
        throw std::runtime_error("Failed to open configuration file.");
    }

    Config config;
    Server current_server;
    Route current_route;
    bool inside_server_block = false;
    bool inside_route_block = false;
    std::string current_route_path;

    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> tokens = tokenize(line);

        if (tokens.empty() || tokens[0][0] == '#') {
            continue;  // Skip empty lines and comments
        }

        if (tokens[0] == "server" && tokens[1] == "{") {
            inside_server_block = true;
            current_server = Server();
        } else if (inside_server_block && tokens[0] == "}") {
            if (inside_route_block) {
                inside_route_block = false;
                current_server.routes[current_route_path] = current_route;
            } else {
                inside_server_block = false;
                config.servers.push_back(current_server);
            }
        } else if (inside_server_block) {
            if (tokens[0] == "listen") {
                current_server.listen = tokens[1];
            } else if (tokens[0] == "server_name") {
                current_server.server_name = tokens[1];
            } else if (tokens[0] == "root") {
                current_server.root = tokens[1];
            } else if (tokens[0] == "index") {
                current_server.index = tokens[1];
            } else if (tokens[0] == "error_page") {
                current_server.error_page = tokens[1];
            } else if (tokens[0] == "client_max_body_size") {
                current_server.client_max_body_size = std::stoi(tokens[1]);
            } else if (tokens[0] == "location") {
                inside_route_block = true;
                current_route_path = tokens[1];
                current_route = Route();  // New route block
            } else if (inside_route_block) {
                if (tokens[0] == "root") {
                    current_route.root = tokens[1];
                } else if (tokens[0] == "allow_methods") {
                    for (size_t i = 1; i < tokens.size(); ++i) {
                        current_route.allowed_methods.push_back(tokens[i]);
                    }
                } else if (tokens[0] == "cgi_pass") {
                    current_route.cgi_path = tokens[1];
                } else if (tokens[0] == "autoindex") {
                    current_route.autoindex = (tokens[1] == "on");
                } else if (tokens[0] == "upload_store") {
                    current_route.upload_store = tokens[1];
                } else if (tokens[0] == "default_file") {
                    current_route.default_file = tokens[1];
                }
            }
        }
    }

    if (inside_server_block || inside_route_block) {
        throw std::runtime_error("Malformed configuration file. Missing closing brace.");
    }

    return config;
}
