#include "../include/Config.hpp"
#include "../include/Server.hpp"
#include "../include/Location.hpp"
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>
#include <vector>      // for std::vector
#include <sys/types.h> // for basic types
#include <sys/socket.h> // for socket-related functions and structures
#include <netinet/in.h> // for sockaddr_in, sockaddr_storage
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>


Config::Config() {
    // std::cout << "Config constructor called" << std::endl;
}

Config::~Config() {
    // std::cout << "Config destructor called" << std::endl;
}

void Config::print_config_parse() const {
std::vector<Server> servers = get_servers();
for (std::vector<Server>::const_iterator serverIt = servers.begin(); serverIt != servers.end(); ++serverIt) {
    serverIt->print_info();
    std::map<std::string, std::vector<Location>> locations = serverIt->getLocations();
    for (std::map<std::string, std::vector<Location>>::const_iterator locIt = locations.begin(); locIt != locations.end(); ++locIt) {
        std::cout << "Location Path: " << locIt->first << std::endl; // Print the path
        const std::vector<Location>& locationVector = locIt->second;
            for (std::vector<Location>::const_iterator vecIt = locationVector.begin(); vecIt != locationVector.end(); ++vecIt) {
                vecIt->printInfo(); // Print information about the location
            }
}
}
std::cout << "---------------------------" << std::endl;
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

void print_tokens(const std::vector<std::string>& tokens) {
    std::cout << "Tokens: ";
    for (size_t i = 0; i < tokens.size(); ++i) {
        std::cout << tokens[i];
        if (i != tokens.size() - 1) {
            std::cout << ", ";  // Add a comma between tokens for clarity
        }
    }
    std::cout << std::endl;  // End with a newline
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
	// std::cout << "HERE" << std::endl;
	// std::cout << "HERE" << std::endl;
    // print_tokens(tokens);
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
    Location new_location;
    bool inside_server_block = false;
    bool inside_location_block = false;
    while (std::getline(file, line)) {
        std::vector<std::string> tokens = tokenize(line);
        
        if (tokens.empty()) continue;

        if (tokens[0] == "server" && tokens[1] == "{") {
            inside_server_block = true;
            continue;
        }
        if (inside_server_block && tokens[0] == "location" && tokens[2] == "{") {
            inside_location_block = true;   
			current_server.set_location(tokens[1], new_location);
            continue;
        }
        if (inside_location_block && tokens[0] == "}") {
            inside_location_block = false;
            current_server.set_location(tokens[1], new_location); // Reset for next location
			new_location = Location();
            continue;
        }
        if (inside_server_block && tokens[0] == "}") {
            inside_server_block = false;
            servers.push_back(current_server);
            current_server = Server(); // Reset for next server block
            continue;
        }
        // Now handle key-value pairs inside blocks
        if (inside_server_block && !inside_location_block) {
            if (tokens.size() >= 2) {
                std::string key = tokens[0];
                std::string value = tokens[1];

                if (key == "listen") {
                    current_server.set_port_string(value);
					// current_server.set_port_char(value);
                } else if (key == "root") {
                    current_server.set_root(value);
                } else if (key == "server_name") {
                    current_server.set_server_name(value);
                } else if (key == "index") {
                    current_server.set_index(value);
                } else if (key == "methods") {
					std::vector<std::string> methods;
					for (size_t i = 1; i < tokens.size(); i++) {
						methods.push_back(tokens[i]);
					}
                    current_server.set_allowed_methods(methods);
                }
                if (key == "error_page") {
                    std::vector<std::string> errorPages;
                    for (size_t i = 1; i < tokens.size(); i++) {
                        errorPages.push_back(tokens[i]);
                    }
                    current_server.set_error_page(errorPages);
                }
            }
        }
        if (inside_location_block) {
            if (tokens.size() >= 2) {
                std::string key = tokens[0];
                std::string value = tokens[1];

                if (key == "root") {
                    new_location.setRoot(value);
                } else if (key == "autoindex") {
                    new_location.setAutoindex(value == "on");
                } else if (key == "cgi_pass") {
                    new_location.setCgiPass(value);
                }
				else if (key == "methods") {
					std::vector<std::string> methods;
					for (size_t i = 1; i < tokens.size(); i++) {
						methods.push_back(tokens[i]);
					}
					new_location.setAllowedMethods(methods);
				}
            }
        }
    }
    return servers;
}

/*Function to run the server and handle incoming connections and data
- pfds: Vector to hold poll file descriptors
- listener: File descriptor for the server listening socket
- poll_test: Result of poll() function to check file descriptors*/

int Config::add_poll_fds() {
    std::vector<struct pollfd> pfds;
    std::map<int, Server*> fd_to_server_map;
    for (Server& current_server : _servers) {
        current_server.listener_fd = current_server.report_ready(pfds);
		fd_to_server_map[current_server.listener_fd] = &current_server;
    }
    while (true) {
        int poll_test = poll(pfds.data(), pfds.size(), -1);
        if (poll_test == -1) {
            std::cerr << "Poll error" << std::endl;
            return -1;
        }
        for (size_t i = 0; i < pfds.size(); i++) {
            if (pfds[i].revents & POLLIN) {
                int fd = pfds[i].fd;
				for (Server& current_server : _servers) {
//					std::cout << fd << " " << current_server.listener_fd << std::endl;
					if (fd == current_server.listener_fd) {
						current_server.handle_new_connection(fd, pfds, i);
					} 
					else {
						current_server.handle_client_data(pfds, i, fd);
					}
				}
            }
        }
    }
	 // file.close();
}


/*
TODO Create the vector of vectors, iterate like 2d array vector<vector<Client>>
TODO StrReceived will be only for body, separate it from head
TODO handleGet check extension (img. html ..) instead cgi-bin
send fileto SendFile instead of filepath, cause i open it twice
TODO in SEndFile Respond why 404? Don't I check it twice before use
TODO Send should be done in chunks like read and go to poll, change events to POLLOUT somewhere
*/

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
    this->_servers_default = _servers;
	add_poll_fds();
    return 0;
}


