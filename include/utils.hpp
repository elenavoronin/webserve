#pragma once

#include <string>
#include <map>
#include "../include/Server.hpp"
#include "../include/Location.hpp"
#include "../include/Config.hpp"

class Server;
class Location;
class Config;

# define READ 0
# define WRITE 1
# define WRITE_SIZE 10
# define READ_SIZE 10
# define BACKLOG 10

std::string             getStatusMessage(int statusCode);
void                    printConfigParse(Config &config);
void                    printInfo(const Server &server);
