#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
class HttpRequest;
class HttpResponse;

class Client {
	private:
		int 			_clientSocket;
		HttpRequest* 	_HttpRequest;
		HttpResponse* 	_HttpResponse;
		int 			_i;

	public:
		Client();// default to 0 if no parameter is provided
		~Client();
		Client& operator=(const Client& copy);
		Client(const Client& copy);
		void setSocket(int i);
		int getSocket();
		//sendRequest();

		//Getter for _i
		int getI();

		//Setter for _i
		void setI(int i);

		//Getter for _HttpRequest
		HttpRequest* getHttpRequest() const;

		//Getter for _HttpResponse
		HttpResponse* getHttpResponse() const;

		//Setter for _HttpRequest
		void setHttpRequest(HttpRequest* httpRequest);

		//Setter for _HttpResponse
		void setHttpResponse(HttpResponse* httpResponse);
};
