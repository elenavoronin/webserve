#pragma once


#include "HttpRequest.hpp"
class HttpRequest;

class Client {
	private:
		int clientSocket;
	public:
		int i;
		HttpRequest* Http;
		Client();
		~Client();
		Client& operator=(const Client& copy);
		Client(const Client& copy);
		void setSocket(int i);
		int getSocket();
		//sendRequest();
};
