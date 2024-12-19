#include "../include/Config.hpp"

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


/**
 * @brief Initializes and adds poll file descriptors for each server.
 * 
 * This function creates an EventPoll object and iterates over the list of servers, 
 * calling each server's report_ready method to retrieve and set the listener file descriptor. 
 * It then enters the poll loop to handle incoming events.
 */
void Config::addPollFds() {
    EventPoll eventPoll;
    for (Server& current_server : _servers) {
        current_server.listener_fd = current_server.reportReady(eventPoll);
    }
    pollLoop(eventPoll);
}

/**
 * @brief The main event loop for the web server.
 * 
 * This function creates a while loop that runs indefinitely, where it:
 * 1. Updates the event list from the add/remove queues.
 * 2. Calls poll() to block until there is an event.
 * 3. Iterates over the pollfds returned by poll() to handle events.
 * 
 * Events are handled by calling the appropriate handler functions on the servers.
 */
void Config::pollLoop(EventPoll &eventPoll) {
    while (true) {
        // Update the event list from the add/remove queues
        eventPoll.updateEventList();

        std::vector<pollfd> &pfds = eventPoll.getPollEventFd();
        int pollResult = poll(pfds.data(), pfds.size(), -1);

        if (pollResult == -1) {
            throw std::runtime_error("Poll failed!");
        }

        // Iterate over the pollfds to handle events
        for (size_t i = 0; i < pfds.size(); i++) {
            if (pfds[i].revents & POLLIN) {
                int fd = pfds[i].fd;

                for (Server &current_server : _servers) {
                    if (fd == current_server.listener_fd) {
                        // Handle new connection
                        current_server.handleNewConnection(eventPoll);
                    } else {
                        // Handle events for existing connections
                        current_server.handlePollEvent(eventPoll, i);
                    }
                }
            }
        }
    }
}



/**
 * @brief Reads and parses a configuration file.
 * 
 * This function reads the configuration file specified by the parameter
 * config_file and parses it into a list of Server objects. If the file
 * cannot be opened or is empty, it prints an error message and returns -1.
 * Otherwise, it sets the _servers member variable to the list of Server
 * objects, sets the _servers_default member variable to the same list,
 * and calls addPollFds() to set up the event loop.
 * 
 * @param config_file The path to the configuration file to read.
 * @return 0 on success, -1 on error.
 */
int Config::checkConfig(const std::string &config_file) {
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
	addPollFds();
    return 0;
}

const std::vector<Server>&Config::get_servers() const {
    return _servers;
}
