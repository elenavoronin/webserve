
#pragma once
#include <vector>
#include <iostream>
#include "Client.hpp"
#include "HttpRequest.hpp"
#include "Client.hpp"
#include <sstream>
#include <fstream>
#include <map>
#include "Config.hpp"
#include "Location.hpp"
#include <poll.h>
#include <algorithm>


class Client; 
// class Location;
class HttpRequest; 
class Server  {
	private:
		std::string										_port_string;
		// const char* 									_port;
		std::string 									_server_name;
		std::string                 					_root;
		std::string										_index;
        std::vector<std::string>    					_allowed_methods;
		bool                        					_autoindex;
		size_t											_maxBodySize; //TODO to parse in config
        std::string                 					_upload_store;
        std::string                 					_default_file;
		std::string										_host;			   
		std::vector<std::string>						_errorPage;
		std::map<std::string, std::vector<Location>>	_locations;

	public:
		int listener_fd;
		std::vector<struct pollfd> pfds;
		std::vector<Client> clients; //do i need it?
		Server();
		Server(const Server& copy) = default;
		Server& operator=(const Server& copy) = default;
		~Server();

		void run();
		/*listener socket*/
		int 	report_ready(std::vector<struct pollfd> &pfds);
		int		get_listener_socket();
		int 	sendall(int s, char *buf, int *len);
		/*Fd management*/
		void 	add_to_pfds(std::vector<struct pollfd> &pfds, int newfd);
		void 	del_from_pfds(std::vector<struct pollfd> &pfds, int i);
		/*Main loop*/
		void 	handle_new_connection(int listener, std::vector<struct pollfd> &pfds, int i);
		void 	handle_client_data(std::vector<struct pollfd> &pfds, int i, int listener);
		void 	broadcast_message(int sender_fd, char *buf, int received, std::vector<struct pollfd> &pfds, int listener);

		void 	addClient(std::vector<struct pollfd> &pfds, int clientSocket, int i);
		void 	removeClient(std::vector<struct pollfd> &pfds, int i, int clientSocket);
		/*Handle requests*/
//		int 	handleRequest(int clientSocket, std::string request, HttpRequest *Http);
		int processClientRequest(int clientSocket, const std::string& request, HttpRequest* Http);
		int handleGetRequest(int clientSocket, const std::string& path, HttpRequest* Http);
		int handlePostRequest(int clientSocket, const std::string& path, HttpRequest* Http);
		int handleDeleteRequest(int clientSocket, const std::string& path, HttpRequest* Http);
		void sendResponse(int clientSocket, const std::string& response);
		void checkLocations(std::string path);

		//setters
     	void set_server_name(const std::string &server_name) { _server_name = server_name; }
        void set_port_string(const std::string &port) { _port_string = port; }
	//	void set_port_char(std::string &value) {_port = value.c_str(); std::cout << "PORT inside setter " << _port << std::endl;}
        void set_root(const std::string &root) { _root = root; }
        void set_autoindex(bool autoindex) { _autoindex = autoindex; }
        void set_upload_store(const std::string &upload_store) { _upload_store = upload_store; }
        void set_allowed_methods(const std::vector<std::string> &allowed_methods) { _allowed_methods = allowed_methods; }
        void set_default_file(const std::string &default_file) { _default_file = default_file; }

		//getter

        void set_index(const std::string &index) { _index = index; }
		void set_error_page(const std::vector<std::string>& errorPages) {
    		_errorPage = errorPages; }
	    //for debugging only

        void print_info() const {
        std::cout << "Server Name: " << _server_name << std::endl;
        std::cout << "Port: " << _port_string << std::endl;
        std::cout << "Root: " << _root << std::endl;
        std::cout << "Index: " << _index << std::endl;
        std::cout << "Allowed Methods: ";
       for (std::vector<std::string>::const_iterator it = _allowed_methods.begin(); it != _allowed_methods.end(); ++it) {
    			std::cout << *it << " ";
        	}
        std::cout << std::endl;
		std::cout << "Error Pages: ";
    	for (std::vector<std::string>::const_iterator it = _errorPage.begin(); it != _errorPage.end(); ++it) {
    		std::cout << *it << " ";
    	}
    	std::cout << std::endl;
		}

		void set_location(const std::string& path, const Location& location) {
			_locations[path].push_back(location);
		}
		std::map<std::string, std::vector<Location>> getLocations() const {
			return _locations;
		}

		std::string getPortStr() const {return this->_port_string;}
		std::string getIndex() const {return this->_index;}
		std::string getServer_name() const {return this->_server_name;}
		std::string getRoot() const {return this->_root;}
		std::vector<std::string> getAllowed_methods() const {return this->_allowed_methods;}
		bool getAutoindex() const {return this->_autoindex;}
		std::string getUpload_store() const {return this->_upload_store;}
		std::string getDefault_file() const {return this->_default_file;}
		std::string getHost() const {return this->_host;}
		std::vector<std::string> getErrorPage() const {return this->_errorPage;}

		void add_poll_fds(std::vector<struct pollfd> &pfds, std::map<int, Server*> &fd_to_server_map);
};
