#pragma once

#include <string>
#include <map>
#include "../include/Server.hpp"
#include "../include/Location.hpp"
#include "../include/Config.hpp"
#include "../include/Client.hpp"
#include <iostream>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>

class Server;
class Location;
class Config;
class Client;

# define READ 0
# define WRITE 1
# define WRITE_SIZE 10000
# define READ_SIZE 10000
# define BACKLOG 100

std::string             getStatusMessage(int statusCode);
void                    printConfigParse(std::vector<Server> &servers);
void                    printInfoServer(const Server &server);
void                    printInfoLocations(const Location &location);
void                    printTokens(const std::vector<std::string>& tokens);
bool                    isEmpty(const Location& location);
void                    printClientsVector(const std::vector<Client>& Clients);
std::string             generateDirectoryListing(const std::string &directoryPath, const std::string &requestPath);