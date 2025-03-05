#pragma once

#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>
#include "../include/utils.hpp"
#include "../include/Location.hpp"
#include "EventPoll.hpp"
#include "../include/Server.hpp"
#include "../include/Location.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <filesystem>
#include <sys/stat.h>
#include <chrono>
#include <ctime>
#include <iomanip>



class Server;
struct defaultServer;

class Config {
    private:
        EventPoll                   _eventPoll;
        std::vector<Server>         _servers;
        std::vector<defaultServer>   _defaultServers;

    public:
        Config();
        Config(const Config& other) = default;
        Config& operator=(const Config& other) = default;
        ~Config();

        int                             checkConfig(const std::string &config_file);
		std::vector<std::string>        tokenize(const std::string &line);
        void                            parseLocationTokens(const std::vector<std::string>& tokens, Location& newLocation);
		bool                            isFileEmpty(const std::string& fileName);
		std::vector<Server>             parseConfig(std::ifstream &file);
        const std::vector<Server>&      getServers() const {return _servers;}
		void                            addPollFds();
        void                            pollLoop();
        
        bool                            validateParsedLocation(Location& location);
        bool                            validateParsedData(Server &server);
        bool                            validateConfig(std::vector<Server> &servers);
};
