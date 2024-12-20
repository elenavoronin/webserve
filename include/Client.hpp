#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "CGI.hpp"
#include <arpa/inet.h>
#include <netdb.h>
#include "utils.hpp"
#include "EventPoll.hpp"
#include "Server.hpp"
#include <unistd.h>

class HttpRequest;
class HttpResponse;
class CGI;
class Server;

/**
 * @brief The Client class represents a single client connection to the server.
 *
 * This class manages the state and behavior of a client connection, including
 * its socket, associated HTTP request and response objects, and optional CGI handling.
 */
class Client {
	private:
		int 			_clientSocket;
		HttpRequest* 	_HttpRequest;
		HttpResponse* 	_HttpResponse;
		CGI*			_CGI;
		EventPoll&		_eventPoll;
		unsigned long	_responseIndex;

	public:
		Client(int clientSocket, EventPoll& eventPoll);
		~Client();
		Client& 			operator=(const Client& copy);
		Client(const Client& copy) 							= default;
		void 				setSocket(int clientSocket);
		int 				getSocket() const;
		HttpRequest* 		getHttpRequest() const;
		HttpResponse* 		getHttpResponse() const;
		Server* 			getServer() const;
		std::string			getBaseDirectory() const;
		void 				setHttpRequest(HttpRequest* httpRequest);
		void 				setHttpResponse(HttpResponse* httpResponse);
		void 				writeToSocket();
		void 				readFromSocket(Server *server);
		void 				closeConnection(EventPoll &eventPoll);
		void				prepareFileResponse();

		//CGI calling methods
		int 				getCgiRead();
		int 				getCgiWrite();
		void 				startCgi(HttpRequest *request);
		void 				readFromCgi();
		// void 				writeToCgi();
};
	// int processClientRequest(const std::string& request, HttpRequest* Http); ????