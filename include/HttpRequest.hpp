#pragma once

#include <iostream>
#include "Server.hpp"
#include <map>


class HttpRequest {
	private: //Or public or private??????

	public:
		std::map<std::string, std::string> _request;
		std::string _strReceived;
		bool headerReceived = false;
		std::string method; //dont need!
		
		std::string path;
		std::string version;
		std::string header; //? change also in copy constructor
		std::string body; //?
		HttpRequest();
		//HttpRequest(std::string method, std::string path, std::string version);
		~HttpRequest();
		HttpRequest& operator=(const HttpRequest& copy);
		HttpRequest(const HttpRequest& copy);

		void setField(std::string key, std::string value);
		std::string getField(std::string key);
		void readRequest(std::string request);
		std::string trim(std::string& str);
		// int checkErrors();
		int findContentLength(std::string request);
};
