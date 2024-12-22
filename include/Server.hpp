
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
#include "utils.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "HttpResponse.hpp"
#include "CGI.hpp"

class Client; 
// class Location;
class HttpRequest; 
class Server  {
	private:
		std::string										_portString;
		// const char* 									_port;
		std::string 									_serverName;
		std::string                 					_root;
		std::string										_index;
        std::vector<std::string>    					_allowedMethods;
		bool                        					_autoindex;
		size_t											_maxBodySize; //TODO to parse in config
        std::string                 					_uploadStore;
        std::string                 					_defaultFile;
		std::string										_host;			   
		std::vector<std::string>						_errorPage;
		std::map<std::string, std::vector<Location>>	_locations;

	public:
		int listener_fd;							// make private
		bool connection = false;					// make private
		std::vector<struct pollfd> pfds; 			// replace this with eventpoll?
		std::vector<Client> clients;				// make private
		
		Server();
		Server(const Server& copy) = default;
		Server& operator=(const Server& copy) = default;
		~Server();

		void 											run();
		/*listener socket*/
		int 											reportReady(EventPoll &eventPoll);
		int												getListenerSocket();
		int 											sendall(int s, char *buf, int *len);
		/*Fd management*/
		void 											delFromPfds(std::vector<struct pollfd> &pfds, int i); // replace pollfd with eventpoll??
		/*Main loop*/
		void 											handleNewConnection(EventPoll &eventPoll);
		void 											handlePollEvent(EventPoll &eventPoll, int i);


		void											eraseClient(int event_fd);
		// void 										broadcastMessage(int sender_fd, char *buf, int received, std::vector<struct pollfd> &pfds, int listener);
		/*Handle requests*/	
		// int 											handleRequest(int clientSocket, std::string request, HttpRequest *Http);
		int 											processClientRequest(Client &client, const std::string& request, HttpRequest* Http);
		int 											handleGetRequest(Client &client, HttpRequest* request);
		int 											handlePostRequest(Client &client, HttpRequest* Http);
		int 											handleDeleteRequest(Client &client, HttpRequest* Http);
		void 											sendResponse(int clientSocket, const std::string& response);
		void 											checkLocations(std::string path);
		void 											sendFileResponse(int clientSocket, const std::string& filepath, int statusCode);
		std::string 									readFileContent(const std::string& filepath);
		void 											sendHeaders(int clientSocket, int statusCode, const std::string& contentType);
		void 											sendBody(int clientSocket, const std::string& body);
		int 											validateRequest(const std::string& method, const std::string& version);

     	void 											setServerName(const std::string &server_name) { _serverName = server_name;}
        void 											setPortString(const std::string &port) { _portString = port;}
        void 											setRoot(const std::string &root) { _root = root;}
        void 											setAutoindex(bool autoindex) { _autoindex = autoindex;}
        void 											setUploadStore(const std::string &upload_store) { _uploadStore = upload_store;}
        void 											setAllowedMethods(const std::vector<std::string> &allowed_methods) { _allowedMethods = allowed_methods;}
        void 											setDefaultFile(const std::string &default_file) { _defaultFile = default_file;}
        void 											setIndex(const std::string &index) { _index = index;}
		void 											setErrorPage(const std::vector<std::string>& errorPages) {_errorPage = errorPages;}
		void 											setLocation(const std::string& path, const Location& location) {_locations[path].push_back(location);}


		std::map<std::string, std::vector<Location>> 	getLocations() const {return _locations;}
		std::string 									getPortStr() const {return this->_portString;}
		std::string 									getIndex() const {return this->_index;}
		std::string 									getServer_name() const {return this->_serverName;}
		std::string 									getRoot() const {return this->_root;}
		std::vector<std::string> 						getAllowed_methods() const {return this->_allowedMethods;}
		bool 											getAutoindex() const {return this->_autoindex;}
		std::string 									getUpload_store() const {return this->_uploadStore;}
		std::string 									getDefault_file() const {return this->_defaultFile;}
		std::string 									getHost() const {return this->_host;}
		std::vector<std::string> 						getErrorPage() const {return this->_errorPage;}
};
