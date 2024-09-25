
#include "Server.hpp"

Server::Server(){}

Server::~Server(){}

Server& Server::operator=(const Server& copy){
    if (this != &copy) {
        // Implement deep copy
    }
    return *this;
}

Server::Server(const Server& copy){}
