#include "../include/Server.hpp"

Server::Server(){
	// this->port = "9034";
    this->port = "8080";
	this->server_name = "localhost"; //should be array of names
}

Server::~Server(){}

Server& Server::operator=(const Server& copy){
    if (this != &copy) {
        // Implement deep copy
    }
    return *this;
}

Server::Server(const Server& copy){}


//THIS IS the test