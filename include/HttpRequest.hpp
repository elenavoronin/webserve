#pragma once

#include <iostream>
#include "Server.hpp"
#include <string>
#include <map>

class HttpRequest {
	private: //Or public or private??????
		std::map<std::string, std::string> 		_request;
		std::string 							_strReceived;
		bool 									_headerReceived = false;
		std::string								_method; //dont need!
		std::string 							_path;
		std::string 							_version;
		std::string 							_header; //? change also in copy constructor
		std::string 							_body; //?

	public:
		HttpRequest();
		//HttpRequest(std::string method, std::string path, std::string version);
		~HttpRequest();
		HttpRequest& operator=(const HttpRequest& copy);
		HttpRequest(const HttpRequest& copy);

		void setField(std::string key, std::string value);
		std::string getField(std::string key);

		std::string getPath() const;

		// Getters and setters for _strReceived
    	std::string& getStrReceived();
    	void clearStrReceived();

    	// Getter and setter for _headerReceived
    	bool isHeaderReceived() const;
    	void setHeaderReceived(bool received);
		
		void readRequest(std::string request);
		std::string trim(std::string& str);
		// int checkErrors();
		int findContentLength(std::string request);
};
