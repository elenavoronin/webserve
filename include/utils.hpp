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
# define READ_SIZE 10000
# define BACKLOG 10

std::string             getStatusMessage(int statusCode);
void                    printConfigParse(std::vector<Server> &servers);
void                    printInfoServer(const Server &server);
void                    printInfoLocations(const Location &location);
void                    printTokens(const std::vector<std::string>& tokens);
bool                    isEmpty(const Location& location);
