
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
#include <filesystem>
#include <stdexcept>
#include <utility>

class Client; 
// class Location;
class HttpRequest; 
class Server  {
	private:
		std::string										_portString;
		std::string 									_serverName;
		std::string                 					_root;
		std::string										_index;
        std::vector<std::string>    					_allowedMethods;
		bool                        					_autoindex;
		size_t											_maxBodySize;
        std::string                 					_uploadStore;   
		std::vector<std::string>						_errorPage;
		std::map<std::string, std::vector<Location>>	_locations;
		int 											_listener_fd;
		std::vector<Client> 							_clients;
		std::pair<int, std::string>						_redirect;
		
	public:
		
		Server();
		Server(const Server& copy) = default;
		Server& operator=(const Server& copy) = default;
		~Server();

		/*listener socket*/
		int 											reportReady(EventPoll &eventPoll);
		int												getListenerSocket();
		/*Main loop*/
		void 											handleNewConnection(EventPoll &eventPoll);
		void 											handlePollEvent(EventPoll &eventPoll, int i, Server& defaultServer);
		void											eraseClient(int event_fd);
		// void 										broadcastMessage(int sender_fd, char *buf, int received, std::vector<struct pollfd> &pfds, int listener);
		/*Handle requests*/	
		int 											processClientRequest(Client &client, const std::string& request, HttpRequest* Http, Server &defaultServer);
		int 											handleGetRequest(Client &client, HttpRequest* request);
		int 											handlePostRequest(Client &client, HttpRequest* Http);
		int 											handleDeleteRequest(Client &client, HttpRequest* request);
		int												handleRedirect(Client& client, HttpRequest& request);
		void 											checkLocations(std::string path, Server &defaultServer);
		void 											sendFileResponse(int clientSocket, const std::string& filepath, int statusCode);
		std::string 									readFileContent(const std::string& filepath);
		void 											sendHeaders(int clientSocket, int statusCode, const std::string& contentType);
		void 											sendBody(int clientSocket, const std::string& body);
		int 											validateRequest(const std::string& method, const std::string& version);
		void											setRedirect(const std::string& statusCode, const std::string& redirectPath);
		void 											setServerName(const std::string &server_name) { _serverName = server_name;}
        void 											setPortString(const std::string &port) { _portString = port;}
        void 											setRoot(const std::string &root) { _root = root;}
        void 											setMaxBodySize(const size_t &maxBodySize) { _maxBodySize = maxBodySize;}
        void 											setAutoindex(bool autoindex) { _autoindex = autoindex;}
        void 											setUploadStore(const std::string &upload_store) { _uploadStore = upload_store;}
        void 											setAllowedMethods(const std::vector<std::string> &AllowedMethods) { _allowedMethods = AllowedMethods;}
        void 											setIndex(const std::string &index) { _index = index;}
		void 											setErrorPage(const std::vector<std::string>& errorPages) {_errorPage = errorPages;}
		void 											setLocation(const std::string& path, const Location& location) {_locations[path].push_back(location);}
		void											setListenerFd(int listener_fd) {_listener_fd = listener_fd;}

		std::string										extractBoundary(const std::string& contentType);
		std::vector<std::string>						splitMultipartBody(const std::string& requestBody, const std::string& boundary);
		std::string										extractFilename(const std::string& headers);
		void											processMultipartPart(const std::string& part);
		void											saveUploadedFile(const std::string& filePath, const std::string& part, size_t dataStart);


		std::map<std::string, std::vector<Location>> 	getLocations() const {return _locations;}
		std::string 									getPortStr() const {return this->_portString;}
		std::string 									getIndex() const {return this->_index;}
		std::string 									getServerName() const {return this->_serverName;}
		size_t		 									getMaxBodySize() const {return this->_maxBodySize;}
		std::string 									getRoot() const {return this->_root;}
		std::vector<std::string> 						getAllowedMethods() const {return this->_allowedMethods;}
		bool 											getAutoindex() const {return this->_autoindex;}
		std::string 									getUploadStore() const {return this->_uploadStore;}
		std::vector<std::string> 						getErrorPage() const {return this->_errorPage;}
		int												getListenerFd() const {return this->_listener_fd;}
		std::pair<int, std::string>						getRedirect() const { return this->_redirect;}
};
