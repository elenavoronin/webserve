#pragma once

#include <iostream>
#include "Server.hpp"
#include <string>
#include <map>

/**
 * @brief The HttpRequest class manages the parsing and storage of HTTP requests.
 *
 * This class provides functionality to parse HTTP request headers and bodies,
 * store key-value pairs of request fields, and determine request states such as
 * whether headers have been fully received.
 * @todo  make _readyToSendBack private
 */
class HttpRequest {

	private:
		std::map<std::string, std::string> 			_request;      						// Stores parsed HTTP request fields (key-value pairs)
		std::string 								_strReceived;						// Raw HTTP request string received from the client.
		bool 										_headerReceived = false;			// Flag indicating whether the request headers have been fully received.

	public:
		bool 										_readyToSendBack = false; // Flag indicating whether the request is ready to send a response.

		HttpRequest();
		~HttpRequest();
		HttpRequest& operator=(const HttpRequest& copy) = default;
		HttpRequest(const HttpRequest& copy) = default;

		void 				setField(std::string key, std::string value);
		std::string 		getField(std::string key);
		std::string& 		getStrReceived();
		void 				setStrReceived(std::string input);
		void 				clearStrReceived();
		bool 				isHeaderReceived() const;
		void 				setHeaderReceived(bool received);
		void 				readRequest(std::string request);
		std::string 		trim(std::string& str);
		int 				findContentLength(std::string request);
};
	//std::string _bodyReceived;// Raw HTTP request string
