#pragma once

#include <iostream>
#include "Server.hpp"

class HttpRequest {
	private:
	public:
		std::string method;
		
		std::string path;
		std::string version;

		std::string header; //?
		std::string body; //?
		
		HttpRequest(std::string method, std::string path, std::string version);
		~HttpRequest();
		HttpRequest& operator=(const HttpRequest& copy);
		HttpRequest(const HttpRequest& copy);


};
