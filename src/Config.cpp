#include "../include/Config.hpp"

Config::Config() {}

Config::~Config() {}

/**
 * @brief Removes any comments from a line (starting with '#') and trims leading
 *        and trailing whitespace.
 * 
 * @param line The line to remove comments and trim.
 * 
 * @return The line with comments removed and trimmed.
 */
std::string removeCommentsAndTrim(const std::string& line) {
    std::size_t commentPos = line.find('#');
    std::string trimmed = (commentPos != std::string::npos) ? line.substr(0, commentPos) : line;
    
    // Remove leading and trailing whitespace
    std::size_t first = trimmed.find_first_not_of(" \t");
    std::size_t last = trimmed.find_last_not_of(" \t");
    
    return (first == std::string::npos) ? "" : trimmed.substr(first, last - first + 1);
}

/**
 * @brief Splits a line into tokens.
 *
 * Skips empty or comment-only lines and splits the line into tokens,
 * treating '{' and '}' as separate tokens (to mark the start and end of blocks).
 * 
 * @param line The line to tokenize.
 * 
 * @return A vector of tokens in the given line.
 */
std::vector<std::string> Config::tokenize(const std::string &line) {
    std::vector<std::string> tokens;
    std::string cleanLine = removeCommentsAndTrim(line);
    
    if (cleanLine.empty()) {
        return tokens; // Skip empty or comment-only lines
    }

    std::stringstream ss(cleanLine);
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

/**
 * @brief Checks if a file is empty.
 *
 * Opens a file in binary mode and checks if the file size is 0.
 *
 * @param fileName The name of the file to check.
 *
 * @return true if the file is empty, false if it is not.
 */
bool Config::isFileEmpty(const std::string& fileName) {
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    return file.tellg() == 0;
}

void Config::parseLocationTokens(const std::vector<std::string>& tokens, Location& newLocation)

{
    if (tokens.size() >= 2) {
        std::string key = tokens[0];
        std::string value = tokens[1];

        if (key == "root") {
            newLocation.setRoot(value);
        } else if (key == "index") {
            newLocation.setIndex(value);
        } else if (key == "autoindex") {
            newLocation.setAutoindex(value == "on");
        } else if (key == "cgi_pass") {
            newLocation.setCgiPass(value);
        }  else if (key == "cgi_extension") {
            newLocation.setCgiExtension(value);
        } else if (key == "cgi_path") {
            newLocation.setCgiPath(value);
        } else if (key == "redirect") {
            newLocation.setRedirect(value);
        } else if (key == "max_body_size") {
            value = value.substr(0, value.size() - 1);
            size_t maxSize = std::stoul(value);
            newLocation.setMaxBodySize(maxSize);
        } else if (key == "methods") {
            std::vector<std::string> methods;
            for (size_t i = 1; i < tokens.size(); i++) {
                methods.push_back(tokens[i]);
            }
            newLocation.setAllowedMethods(methods);
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
 * objects, sets the _serversDefault member variable to the same list,
 * and calls addPollFds() to set up the event loop.
 * 
 * @param config_file The path to the configuration file to read.
 * @return 0 on success, -1 on error.
 */
std::vector<Server> Config::parseConfig(std::ifstream &file) {
    std::vector<Server> servers;
    Server currentServer;
    std::string line;
    currentServer.getLocations()["keys"].reserve(100); // Pre-allocate space for expected number of locations.
    Location newLocation;
    std::vector<std::string> errorPages;
    bool insideServerBlock = false;
    bool insideLocationBlock = false;
    bool locationComplete = false;

    while (std::getline(file, line)) {
        std::vector<std::string> tokens = tokenize(line);
        
        if (tokens.empty()) continue;

        if (tokens[0] == "server" && tokens[1] == "{") {
            insideServerBlock = true;
            continue;
        }
        if (insideServerBlock && tokens[0] == "location" && tokens[2] == "{") {
            insideLocationBlock = true;   
			currentServer.setLocation(tokens[1], newLocation);
            continue;
        }
        if (insideLocationBlock && tokens[0] == "}" && locationComplete) {
            insideLocationBlock = false;
            locationComplete = false;
            currentServer.setLocation(tokens[1], newLocation);
            newLocation = Location();
            continue;
        }
        if (insideServerBlock && tokens[0] == "}") {
            insideServerBlock = false;
            servers.push_back(currentServer);
            currentServer = Server(); // Reset for next server block
            continue;
        }
        // Now handle key-value pairs inside blocks
        if (insideServerBlock && !insideLocationBlock) {
            if (tokens.size() >= 2) {
                std::string key = tokens[0];
                std::string value = tokens[1];

                if (key == "listen") {
                    currentServer.setPortString(value);
                } else if (key == "root") {
                    currentServer.setRoot(value);
                } else if (key == "server_name") {
                    currentServer.setServerName(value);
                } else if (key == "index") {
                    currentServer.setIndex(value);
                } else if (key == "max_body_size") {
                    value = value.substr(0, value.size() - 1);
                    size_t maxSize = std::stoul(value);
                    currentServer.setMaxBodySize(maxSize);
                }else if (key == "methods") {
					std::vector<std::string> methods;
					for (size_t i = 1; i < tokens.size(); i++) {
						methods.push_back(tokens[i]);
					}
                    currentServer.setAllowedMethods(methods);
                }
                if (key == "error_page") {
                        errorPages.push_back(tokens[2]);
                    }
                    currentServer.setErrorPage(errorPages);
                }
            }
        if (insideLocationBlock) {
            parseLocationTokens(tokens, newLocation);
            if (isEmpty(newLocation) == false)
                locationComplete = true;
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
    for (Server& currentServer : _servers) {
        currentServer.listener_fd = currentServer.reportReady(eventPoll);
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
// void Config::pollLoop() {
void Config::pollLoop(EventPoll &eventPoll) {
    while (true) {
        // Update the event list from the add/remove queues
        eventPoll.updateEventList();
        // _eventPoll.updateEventList();

        std::vector<pollfd> &pfds = eventPoll.getPollEventFd();
        // std::vector<pollfd> &pfds = _eventPoll.getPollEventFd();
        int pollResult = poll(pfds.data(), pfds.size(), -1);

        if (pollResult == -1) {
            throw std::runtime_error("Poll failed!");
        }

        // Iterate over the pollfds to handle events
        for (size_t i = 0; i < pfds.size(); i++) {
            if (pfds[i].revents & POLLIN || pfds[i].revents & POLLOUT || pfds[i].revents & POLLHUP || pfds[i].revents & POLLRDHUP) {
                int fd = pfds[i].fd;

                for (Server &currentServer : _servers) {
                    if (fd == currentServer.listener_fd) {
                        // Handle new connection
                        currentServer.handleNewConnection(eventPoll);
                        // currentServer.handleNewConnection(_eventPoll);
                    } else {
                        // Handle events for existing connections
                        currentServer.handlePollEvent(eventPoll, i);
                        // currentServer.handlePollEvent(_eventPoll, i);
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
 * objects, sets the _serversDefault member variable to the same list,
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
    _servers = parseConfig(file);
    printConfigParse(_servers);
    this->_serversDefault = _servers;
	addPollFds();
    return 0;
}
