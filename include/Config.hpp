#pragma once

#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>
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
#include "../include/utils.hpp"
#include "../include/utils.hpp"

class Server;

class Config {
    private:
        EventPoll              _eventPoll;
        std::vector<Server>    _servers;
        std::vector<Server>    _serversDefault;

    public:
        Config();
        Config(const Config& other) = default;
        Config& operator=(const Config& other) = default;
        ~Config();

        int                             checkConfig(const std::string &config_file);
		std::vector<std::string>        tokenize(const std::string &line);
		bool                            isFileEmpty(const std::string& fileName);
		std::vector<Server>             parseConfig(std::ifstream &file);
		void                            printServers() const;
        const std::vector<Server>&      getServers() const;
		void                            printConfigParse() const;
		void                            addPollFds();
        void                            pollLoop(EventPoll &eventPoll);
};
