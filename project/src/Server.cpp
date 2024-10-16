
#include "../include/Server.hpp"
#include <vector>

Server::Server(){
	// this->_port = "9034";
	// this->_server_name = "localhost"; //should be array of names
}

Server::~Server(){

}

Server::Server(const Server& copy) {
    _server_name = copy._server_name;
    _port = copy._port;
    _root = copy._root;
    _autoindex = copy._autoindex;
    _cgi_path = copy._cgi_path;
    _upload_store = copy._upload_store;
    _allowed_methods = copy._allowed_methods;
    _default_file = copy._default_file;
}


