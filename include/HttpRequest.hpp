#pragma once

#include <iostream>
#include "Server.hpp"
#include <string>
#include <map>

class HttpRequest {

	private:
		std::map<std::string, std::string> _request;      // Stores parsed HTTP request fields (key-value pairs)
		std::string _strReceived;                        // Raw HTTP request string
		bool _headerReceived = false;                    // Indicates if headers are fully received

	public:
		HttpRequest();                                   // Constructor to initialize request object
		~HttpRequest();                                  // Destructor
		HttpRequest& operator=(const HttpRequest& copy); // Assignment operator for deep copy
		HttpRequest(const HttpRequest& copy);            // Copy constructor

		// Methods to set and get fields
		void setField(std::string key, std::string value);    // Set a key-value pair in the request
		std::string getField(std::string key);               // Retrieve value for a specific key

		// Getters and setters for _strReceived
		std::string& getStrReceived();                       // Access raw received request string
		void clearStrReceived();                             // Clear the raw request string

		// Getter and setter for _headerReceived
		bool isHeaderReceived() const;                       // Check if headers are fully received
		void setHeaderReceived(bool received);               // Set the header received status

		// Request parsing and utility methods
		void readRequest(std::string request);               // Parse the raw HTTP request string
		std::string trim(std::string& str);                  // Trim whitespace from a string
		int findContentLength(std::string request);          // Get the Content-Length from the request
};
