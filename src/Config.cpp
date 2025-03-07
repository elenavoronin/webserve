#include "../include/Config.hpp"

Config::Config() {}

Config::~Config() {}

/**
 * @brief Validates the configuration of a given vector of Server objects.
 *
 * This function returns true if the configuration is valid, and false otherwise.
 * A valid configuration is one in which no two servers have the same name and port.
 * If there are two servers with the same port, the server with the name "localhost"
 * has precedence and is set to be on, and the other server is set to be off.
 */
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
                Server server1 = *it1;
                if (server2.getServerName() == "localhost")
                    server1.setOnOff(false);
                else
                    server2.setOnOff(false);
            }
            ++it2;
        }
    }
    return true;
}

/**
 * @brief Validates and adjusts the configuration of a Location object.
 *
 * This function ensures that the redirect status code of the Location object
 * is either 301, 302 or 0. If the autoindex is not set to "on", it defaults
 * to "off". Additionally, if the maximum body size is not set, it is assigned
 * a default value of 1000000.
 *
 * @param location A reference to the Location object that is being validated and adjusted.
 * 
 * @return true if the Location configuration is valid and adjustments have been made, false otherwise.
 */

bool Config::validateParsedLocation(Location& location) {
    if (location.getRedirect().first != 301 && location.getRedirect().first != 302 && location.getRedirect().first != 0)
        return false;
    if (location.getAutoindex() != "on")
        location.setAutoindex("off");
    if (!location.getMaxBodySize())
        location.setMaxBodySize(1000000);
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
        std::string path = "./www/html/upload/";
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
    if (server.getPortStr().size() < 4)
        return false;
    if (server.getRoot().empty())
        return false;
    if (server.getIndex().empty())
        return false;
    if (server.getRedirect().first != 0 && server.getRedirect().first != 301 && server.getRedirect().first != 302)
        return false;
    if (server.getAutoindex() != "on")
        server.setAutoindex("off");
    server.setOnOff(true);
    if (!server.getMaxBodySize())
        server.setMaxBodySize(1000000);
    
    defaultServer _defaultS;
    _defaultS._allowedMethods = server.getAllowedMethods();
    _defaultS._autoindex = server.getAutoindex();
    _defaultS._index = server.getIndex();
    _defaultS._maxBodySize = server.getMaxBodySize();
    _defaultS._portString = server.getPortStr();
    _defaultS._redirect = server.getRedirect();
    _defaultS._root = server.getRoot();
    _defaultS._serverName = server.getServerName();
    _defaultS._uploadStore = server.getUploadStore();
    _defaultS._errorPages = server.getErrorPages();

    server.setDefaultServer(_defaultS);

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
    printTokens(tokens);
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

/**
 * @brief Parses a vector of tokens into a Location object.
 *
 * This function takes a vector of strings and a Location object as parameters.
 * It then iterates over the vector and calls the appropriate setter functions
 * of the Location object to fill in its fields.
 *
 * @param tokens The vector of strings to parse.
 * @param newLocation The Location object to fill in.
 */
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
            newLocation.setAutoindex(value);
        } else if (key == "upload_path") {
            newLocation.setUploadPath(value);
        } else if (key == "cgi_pass") {
            newLocation.setCgiPass(value);
        }  else if (key == "cgi_extension") {
            newLocation.setCgiExtension(value);
        } else if (key == "cgi_path") {
            newLocation.setCgiPath(value);
        } else if (key == "error_page" && !tokens[1].empty() && !tokens[2].empty()) {
            newLocation.setErrorPage(tokens[1], tokens[2]);
        } else if (key == "return") {
            if (!tokens[1].empty() && !tokens[2].empty())
                newLocation.setRedirect(tokens[1], tokens[2]);
            else
                newLocation.setRedirect("0", "");
        } else if (key == "max_body_size") {
            value = value.substr(0, value.size());
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
    bool insideServerBlock = false;
    bool insideLocationBlock = false;

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
            if (insideLocationBlock && tokens[0] == "}" ) {
                insideLocationBlock = false;
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
                if (validateParsedData(currentServer)) {
                    servers.push_back(currentServer);
                    _defaultServers.push_back(currentServer.getDefaultServer());
                }
                else {
                    throw std::runtime_error("Error in config file: Invalid server block detected.");
                }
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
                    } else if (key == "upload_path") {
                        currentServer.setUploadStore(value); 
                    } else if (key == "autoindex") {
                        currentServer.setAutoindex(value); 
                    } else if (key == "return") {
                        if (!tokens[1].empty() && !tokens[2].empty())
                            currentServer.setRedirect(tokens[1], tokens[2]);
                        else
                            currentServer.setRedirect("0", "");
                    } else if (key == "max_body_size") {
                        value = value.substr(0, value.size());
                        size_t maxSize = std::stoul(value);
                        currentServer.setMaxBodySize(maxSize);
                    }else if (key == "methods") {
                        std::vector<std::string> methods;
                        for (size_t i = 1; i < tokens.size(); i++) {
                            methods.push_back(tokens[i]);
                        }
                        currentServer.setAllowedMethods(methods);
                    }
                    if (key == "error_page" && !tokens[1].empty() && !tokens[2].empty()) {
                            currentServer.setErrorPage(tokens[1], tokens[2]);
                        }
                    }
                }
            if (insideLocationBlock) {
                parseLocationTokens(tokens, newLocation);
            }
        }
    }  catch (const std::exception &e) {
        throw std::runtime_error("Error in config file: " + std::string(e.what()));
    }
    file.close();
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
 * @brief Checks for CGI script timeouts and handles them.
 *
 * This function checks every client in the server's client list for CGI script timeouts.
 * If a timeout is detected, it handles the error by either terminating the CGI process or
 * sending a 500 error response to the client. If a timeout is found, the function returns -1,
 * otherwise it returns 0.
 *
 * @param eventPoll The EventPoll object to use for handling events.
 * @param fd The file descriptor of the client to check.
 * @return -1 if a timeout is detected, 0 otherwise.
 */
int Server::timeout_check(EventPoll &eventPoll, int fd){
	(void)fd;
	(void)eventPoll;
	for (Client &c : _clients){
			auto now_time = std::chrono::system_clock::now();
			std::time_t start_time = std::chrono::system_clock::to_time_t(c.getStartTime());
			auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now_time - c.getStartTime()).count();
			if (elapsed > 3 && start_time != 0) {  // Timeout threshold (5 seconds)
				if (c.getCGI()) {
					handleCgiError(&c, 504);
					c.setStartTime(std::chrono::system_clock::now());
					return -1;
				}
				else {
					sendErrorResponse(c, 500);
					c.setStartTime(std::chrono::system_clock::now());
					return -1;
				}
			}
	}
	return 0;
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
        int pollResult = poll(pfds.data(), pfds.size(), 3000);
        if (pollResult == -1) {
            throw std::runtime_error("Poll failed!");
        }

        for (size_t i = 0; i < pfds.size(); i++) {
			for (Server &currentServer : _servers) {
					if (currentServer.timeout_check(_eventPoll, pfds[i].fd) == -1)
						break ;
				}
            if (pfds[i].revents & POLLERR) {
				if (pfds[i].revents & POLLERR) {
					int err = 0;
					socklen_t len = sizeof(err);
					getsockopt(pfds[i].fd, SOL_SOCKET, SO_ERROR, &err, &len);
					close(pfds[i].fd);
    				_eventPoll.ToremovePollEventFd(pfds[i].fd, pfds[i].events);
					break ;
				}
			}
            if (pfds[i].revents & POLLIN || pfds[i].revents & POLLOUT || pfds[i].revents & POLLHUP || pfds[i].revents & POLLRDHUP) {
                int fd = pfds[i].fd;
                Server* defaultServer = nullptr;
                Server* activeServer = nullptr;

                for (Server &currentServer : _servers) {
                    if (!defaultServer && currentServer.getOnOff() == true)
                        defaultServer = &currentServer;
                    if (currentServer.getOnOff() == true)
                        activeServer = &currentServer;
                    Server* selectedServer = activeServer ? activeServer : defaultServer;
                    if (fd == currentServer.getListenerFd()) {
                        // Handle new connection
                        selectedServer->handleNewConnection(_eventPoll);
                    } 
                    else {
                        // Handle events for existing connections
                        selectedServer->handlePollEvent(_eventPoll, i, selectedServer->getDefaultServer(), _defaultServers);
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
        addPollFds();
    } catch (const std::exception &e) {
        std::cerr << "Configuration error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
