#pragma once

// class Client;

#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>
#include "../include/Location.hpp"
#include "EventPoll.hpp"

class Server;

class Config {
    private:
        EventPoll              _eventPoll;
        std::vector<Server>    _servers;
        std::vector<Server>    _servers_default;

    public:
        Config();
        Config(const Config& other) = default;
        Config& operator=(const Config& other) = default;
        ~Config();
        int check_config(const std::string &config_file);
		std::vector<std::string> tokenize(const std::string &line);
		bool isFileEmpty(const std::string& fileName);
		std::vector<Server> parse_config(std::ifstream &file);
		void print_servers() const;
        const std::vector<Server>& get_servers() const {
            return _servers;
        }
		void print_config_parse() const;

        //TODO create Poll class
		void addPollFds();
        void pollLoop(EventPoll &eventPoll);
};
