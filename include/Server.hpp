
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
#include <chrono>

struct defaultServer {
	std::string 						_serverName;
	std::string 						_portString;
	std::string 						_root;
	std::string 						_index;
	std::string 						_autoindex;
	std::string 						_uploadStore;
	std::map<int, std::string> 			_errorPages;
	std::vector<std::string> 			_allowedMethods;
	size_t 								_maxBodySize;
	std::pair<int, std::string>			_redirect;
};

class Client; 
class HttpRequest; 
class Server  {
	private:
		bool											_on;
		std::string										_portString;
		std::string 									_serverName;
		std::string                 					_root;
		std::string										_index;
        std::vector<std::string>    					_allowedMethods;
		std::string                       				_autoindex;
		size_t											_maxBodySize;
        std::string                 					_uploadStore;
		std::map<int, std::string>						_errorPages;
		std::map<std::string, std::vector<Location>>	_locations;
		int 											_listener_fd;
		std::vector<Client> 							_clients;
		std::pair<int, std::string>						_redirect;
		defaultServer									_defaultServer;

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
		void											handlePollEvent(EventPoll &eventPoll, int i, defaultServer defaultS, std::vector<defaultServer> servers);
		void											eraseClient(int event_fd);
		/*Handle requests*/	
		int 											processClientRequest(Client &client, const std::string& request, HttpRequest* Http, defaultServer defaultS, std::vector<defaultServer> servers);
		int 											handleGetRequest(Client &client, HttpRequest* request);
		int 											handlePostRequest(Client &client, HttpRequest* Http);
		int 											handleDeleteRequest(Client &client, HttpRequest* request);
		int												handleRedirect(Client& client);
		void 											checkLocations(std::string path, defaultServer defaultServer);
		std::string 									readFileContent(const std::string& filepath);
		int 											validateRequest(const std::string& method, const std::string& version);
		void											setRedirect(const std::string& statusCode, const std::string& redirectPath);
		void 											setServerName(const std::string &server_name) { _serverName = server_name;}
        void 											setPortString(const std::string &port) { _portString = port;}
        void 											setRoot(const std::string &root) { _root = root;}
        void 											setMaxBodySize(const size_t &maxBodySize) { _maxBodySize = maxBodySize;}
        void 											setAutoindex(std::string autoindex) { _autoindex = autoindex;}
        void 											setUploadStore(const std::string &upload_store) { _uploadStore = upload_store;}
        void 											setAllowedMethods(const std::vector<std::string> &AllowedMethods) { _allowedMethods = AllowedMethods;}
        void 											setIndex(const std::string &index) { _index = index;}
		void 											setErrorPage(const std::string statusCode, const std::string& path) { int code = std::stoi(statusCode); _errorPages[code] = path;}
		void 											setLocation(const std::string& path, const Location& location) {_locations[path].push_back(location);}
		void											setListenerFd(int listener_fd) {_listener_fd = listener_fd;}
		void											setOnOff(bool on) {_on = on;}
		void											setDefaultServer(defaultServer defaultServer) {_defaultServer = defaultServer;}	
		void											setErrorPages(const std::map<int, std::string>& errorPages) {_errorPages = errorPages;}

		std::string										extractBoundary(Client &client, const std::string& contentType);
		std::vector<std::string>						splitMultipartBody(const std::string& requestBody, const std::string& boundary);
		std::string										extractFilename(const std::string& headers);
		int												processMultipartPart(const std::string& part, const std::string& uploadPath);
		void											saveUploadedFile(const std::string& filePath, const std::string& part, size_t dataStart);
		int												sendErrorResponse(Client &client, int statusCode);
		bool											fileExists(const std::string& path);
		void 											ensureUploadDirectoryExists(const std::string& path);
        void 											handleCgiError(Client *client, int statusCode);

        std::map<std::string, std::vector<Location>> 	getLocations() const {return _locations;}
		std::string 									getPortStr() const {return this->_portString;}
		std::string 									getIndex() const {return this->_index;}
		std::string 									getServerName() const {return this->_serverName;}
		size_t		 									getMaxBodySize() const {return this->_maxBodySize;}
		std::string 									getRoot() const {return this->_root;}
		std::vector<std::string> 						getAllowedMethods() const {return this->_allowedMethods;}
		std::string										getAutoindex() const {return this->_autoindex;}
		std::string 									getUploadStore() const {return this->_uploadStore;}
		std::string										getErrorPage(int statusCode) const {try {return _errorPages.at(statusCode); } catch (const std::out_of_range&) {return "";}}
		std::map<int, std::string>						getErrorPages() const {return this->_errorPages;}
		int												getListenerFd() const {return this->_listener_fd;}
		std::pair<int, std::string>						getRedirect() const { return this->_redirect;}
		bool											getOnOff() const {return this->_on;}
		defaultServer									getDefaultServer() const {return this->_defaultServer;}
		void 											checkServer(HttpRequest* HttpRequest, std::vector<defaultServer> servers);

		int												timeout_check(EventPoll &eventPoll, int fd);
};
