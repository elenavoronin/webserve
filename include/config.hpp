#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>

class Config {
    private:

    public:
        Config();
        Config(const Config &other);
        Config &operator=(const Config &other);
        ~Config();
};

struct Route {
    std::string root;
    std::vector<std::string> allowed_methods;
    std::string cgi_path;
    bool autoindex;
    std::string upload_store;
    std::string default_file;
};

struct Server {
    std::string listen;
    std::string server_name;
    std::string root;
    std::string index;
    std::string error_page;
    size_t client_max_body_size;
    std::map<std::string, Route> routes;  // Maps a URL path to a Route config
};

struct Config {
    std::vector<Server> servers;  // Handle multiple servers
};