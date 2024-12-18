#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "CGI.hpp"
#include <arpa/inet.h>
#include <netdb.h>
#include "utils.hpp"
#include "EventPoll.hpp"
#include "Server.hpp"

class HttpRequest;
class HttpResponse;
class CGI;
class Server;

class Client {
	private:
		int 			_clientSocket;
		HttpRequest* 	_HttpRequest;
		HttpResponse* 	_HttpResponse;
		CGI*			_CGI;
		int				_responseIndex;

	public:
		Client(int clientSocket);
		~Client();
		Client& 			operator=(const Client& copy) 	= default;
		Client(const Client& copy) 							= default;
		void 				setSocket(int clientSocket);
		int 				getSocket() const;
		//sendRequest();

		//Getter for _HttpRequest
		HttpRequest* 		getHttpRequest() const;

		//Getter for _HttpResponse
		HttpResponse* 		getHttpResponse() const;

		//Setter for _HttpRequest
		void 				setHttpRequest(HttpRequest* httpRequest);

		//Setter for _HttpResponse
		void 				setHttpResponse(HttpResponse* httpResponse);

		void 				writeToSocket();

		// int processClientRequest(const std::string& request, HttpRequest* Http); ????
		void 				readFromSocket(Server *server);
		
		void 				closeConnection(EventPoll &eventPoll);

		//CGI calling methods
		int 				getCgiRead();
		int 				getCgiWrite();
		void 				startCgi(HttpRequest *request);
		void 				readFromCgi();
		// void 				writeToCgi();

};
