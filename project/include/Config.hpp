#pragma once

class Server;
class Client;

#include <vector>
#include <iostream>

class Config {
    private:
        std::vector<Server>    _servers;

    public:
        Config();
        ~Config();
        int check_config(const std::string &config_file);
};