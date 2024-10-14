#pragma once

#include "Server.hpp"

class Client {
	private:
		int clientSocket; //do I even need it?
	public:
		Client();
		~Client();
		Client& operator=(const Client& copy);
		Client(const Client& copy);
		void setSocket(int i);
		int getSocket();
		//sendRequest();
};
