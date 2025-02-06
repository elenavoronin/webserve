#include "../include/Config.hpp"

Config::Config() {}

Config::~Config() {}


bool Config::validateConfig(std::vector<Server> &servers) {
    if (servers.size() < 2)
        return true;
    for (std::vector<Server>::iterator it1 = servers.begin(); it1 != servers.end(); ++it1) {
        for (std::vector<Server>::iterator it2 = it1 + 1; it2 != servers.end();) {
            if (it1->getServerName() == it2->getServerName() && it1->getPortStr() == it2->getPortStr()) {
                return false;
            }
            if (it1->getPortStr() == it2->getPortStr()) {
                Server server2 = *it2;
                server2.setOnOff(false);   //we turn off this server
            }
            ++it2;
        }
    }
    return true;
}


bool Config::validateParsedLocation(Location& location) {
    if (location.getRedirect().first != 301 && location.getRedirect().first != 302 && location.getRedirect().first != 0)
        return false;
    return true;
}

/**
 * @brief Validates the essential fields of a server configuration.
 * 
 * This function checks if the server's configuration has a valid port number,
 * root directory, and index file. The port number must not be empty and must
 * contain only digits. The root directory and index file must not be empty.
 * 
 * @param server A reference to the Server object whose configuration is being validated.
 * 
 * @return true if the server configuration is valid, false otherwise.
 */
bool Config::validateParsedData(Server &server) {
    if (server.getPortStr().empty())
        return false;
    if (server.getUploadStore().empty()) {
        std::string path = "./www/upload/";
        struct stat info;
        server.setUploadStore(path);
        if (stat(path.c_str(), &info) != 0) {    
            if (mkdir(path.c_str(), 0777) == -1) {
            throw std::runtime_error("Failed to create directory: " + path);
            }
        }
    }
    for (char c : server.getPortStr()) {
    if (!std::isdigit(c)) {
        return false;
    } }
    if (server.getRoot().empty())
        return false;
    if (server.getIndex().empty())
        return false;
    if (server.getRedirect().first != 0 && server.getRedirect().first != 301 && server.getRedirect().first != 302)
        return false;
    server.setOnOff(true);
    return true;
}

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
    if (tokens.size() >= 3) {
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
        } else if (key == "return") {
            if (!tokens[1].empty() && !tokens[2].empty())
                newLocation.setRedirect(tokens[1], tokens[2]);
            else
                newLocation.setRedirect("0", "");
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
    currentServer.getLocations()["keys"].reserve(100);
    Location newLocation;
    std::string pathName;
    std::vector<std::string> errorPages;
    bool insideServerBlock = false;
    bool insideLocationBlock = false;
    bool locationComplete = false;

    try {
        while (std::getline(file, line)) {
            std::vector<std::string> tokens = tokenize(line);
            
            if (tokens.empty()) continue;

            if (tokens[0] == "server" && tokens[1] == "{") {
                insideServerBlock = true;
                continue;
            }
            if (insideServerBlock && tokens[0] == "location" && tokens[2] == "{") {
                pathName= tokens[1];
                insideLocationBlock = true;
                continue;
            }
            if (insideLocationBlock && tokens[0] == "}" && locationComplete) {
                insideLocationBlock = false;
                locationComplete = false;
                if (validateParsedLocation(newLocation))
                {
                    currentServer.setLocation(pathName, newLocation);
                    newLocation.clearLocation();
                }
                newLocation = Location();
                continue;
            }
            if (insideServerBlock && tokens[0] == "}") {
                insideServerBlock = false;
                if (validateParsedData(currentServer))
                    servers.push_back(currentServer);
                else {
                    throw std::runtime_error("Error in config file: Invalid server block detected.");
                }
                currentServer = Server(); // Reset for next server block
                errorPages.clear();
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
                    } else if (key == "upload_path") {
                        currentServer.setUploadStore(value); 
                    } else if (key == "return") {
                        if (!tokens[1].empty() && !tokens[2].empty())
                            currentServer.setRedirect(tokens[1], tokens[2]);
                        else
                            currentServer.setRedirect("0", "");
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
                if (!isEmpty(newLocation))
                    locationComplete = true;
            }
        }
    }  catch (const std::exception &e) {
        std::cerr << "Parsing error: " << e.what() << std::endl;
        throw;
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
    for (Server& currentServer : _servers) {
        currentServer.setListenerFd(currentServer.reportReady(_eventPoll));
    }
    pollLoop();
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
void Config::pollLoop() {
    while (true) {
        // Update the event list from the add/remove queues
        _eventPoll.updateEventList();


        std::vector<pollfd> &pfds = _eventPoll.getPollEventFd();
        int pollResult = poll(pfds.data(), pfds.size(), -1);
        // std::cout << "size of pollfds" << pfds.size() << std::endl; 
        if (pollResult == -1) {
            throw std::runtime_error("Poll failed!");
        }
        if (pollResult == 0) {
            throw std::runtime_error("Poll timed out!");
        }

        // Iterate over the pollfds to handle events
        for (size_t i = 0; i < pfds.size(); i++) {
            //std::cout << "Polling on " << pfds.size() << " sockets" << std::endl;

            if (pfds[i].revents & POLLERR) {
				if (pfds[i].revents & POLLERR) {
					int err = 0;
					socklen_t len = sizeof(err);
					getsockopt(pfds[i].fd, SOL_SOCKET, SO_ERROR, &err, &len);
					std::cerr << "Poll error on fd " << pfds[i].fd << " : " << strerror(err) << std::endl;
					close(pfds[i].fd);
    				_eventPoll.ToremovePollEventFd(pfds[i].fd, pfds[i].events);
					break ;
				}
			}
            // If an FD is active but not being handled correctly, close it
            if ((pfds[i].revents & POLLIN || pfds[i].revents & POLLOUT) && isFdStuck(pfds[i].fd)) {
                std::cerr << "[WARNING] FD: " << pfds[i].fd << " is stuck, closing it." << std::endl;
                close(pfds[i].fd);
                _eventPoll.ToremovePollEventFd(pfds[i].fd, pfds[i].events);
                continue; // Skip further processing of this FD
            }


            if (pfds[i].revents & POLLIN || pfds[i].revents & POLLOUT || pfds[i].revents & POLLHUP || pfds[i].revents & POLLRDHUP) {
                int fd = pfds[i].fd;
                Server* defaultServer = nullptr;
                Server* activeServer = nullptr;

                for (Server &currentServer : _servers) {
                    if (!defaultServer)
                        defaultServer = &currentServer;
                    if (fd == currentServer.getListenerFd()) {
                        if (currentServer.getOnOff() == true)
                            activeServer = &currentServer;
                    }
                    Server* selectedServer = activeServer ? activeServer : defaultServer;
                    if (fd == currentServer.getListenerFd()) {
                        // Handle new connection
                        selectedServer->handleNewConnection(_eventPoll);
                    } else {
                        // Handle events for existing connections
                        selectedServer->handlePollEvent(_eventPoll, i, *defaultServer);
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
    try {
        _servers = parseConfig(file);
        if (!validateConfig(_servers)) {
            throw std::runtime_error("Error in config file: Invalid servers.");
            return -1;
        }
        // printConfigParse(_servers);
        addPollFds();
    } catch (const std::exception &e) {
        std::cerr << "Configuration error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
