
#ifndef Client_HPP
#define Client_HPP
#include "Server.hpp"

class Client : public Server{
	private:
		Client();
	
	public:
		~Client();
		Client& operator=(const Client& copy);
		Client(const Client& copy);

		// sendRespond();
};





#endif