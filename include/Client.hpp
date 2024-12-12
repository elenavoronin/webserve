#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "CGI.hpp"

class HttpRequest;
class HttpResponse;
class CGI;

class Client {
	private:
		int 			_clientSocket;
		HttpRequest* 	_HttpRequest;
		HttpResponse* 	_HttpResponse;
		CGI*			_CGI;

	public:
		Client();// default to 0 if no parameter is provided
		~Client();
		Client& operator=(const Client& copy);
		Client(const Client& copy);
		void setSocket(int clientSocket);
		int getSocket();
		//sendRequest();

		//Getter for _HttpRequest
		HttpRequest* getHttpRequest() const;

		//Getter for _HttpResponse
		HttpResponse* getHttpResponse() const;

		//Setter for _HttpRequest
		void setHttpRequest(HttpRequest* httpRequest);

		//Setter for _HttpResponse
		void setHttpResponse(HttpResponse* httpResponse);

		// int processClientRequest(const std::string& request, HttpRequest* Http); ????

		int getCgiRead();
		int getCgiWrite();

		void startCgi(HttpRequest *request);
		void readFromCgi();
		void readFromSocket(Server *server);
};
