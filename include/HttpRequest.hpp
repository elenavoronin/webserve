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
		std::string									_fullPath;
		std::string									_path;
		std::string									_method;
		std::string									_version;
		std::string 								_body;
		std::map<std::string, std::string> 			_headers;
		std::string									_fileName;

	public:
		bool 										_readyToSendBack = false; // Flag indicating whether the request is ready to send a response.

		HttpRequest();
		~HttpRequest();
		HttpRequest& operator=(const HttpRequest& copy) = default;
		HttpRequest(const HttpRequest& copy) = default;

		void 												setField(std::string key, std::string value);
		std::string 										getField(std::string key);
		std::string& 										getStrReceived();
		void 												setStrReceived(std::string input);
		void 												clearStrReceived();
		bool 												isHeaderReceived() const;
		void 												setHeaderReceived(bool received);
		void 												readRequest(std::string request);
		std::string 										trim(std::string& str);
		int 												findContentLength(std::string request);
		void												setPath(std::string path);
		void												setFullPath(std::string path);
		void												setMethod(std::string method);
		void												setVersion(std::string version);
		void 												setBody(std::string& body);

		std::string 										getPath();
		std::string 										getFullPath();
		std::string											getMethod();
		std::string											getVersion();
		std::string 										getBody();
		std::string 										getHeader(const std::string& key);
		std::string											getRawRequest();
		std::string											getPathToDelete(const std::string& rawRequest);
		
		void 												parseHeaders(const std::string& rawRequest);
		void												parseBody(const std::string& rawRequest);
		const std::map<std::string, std::string>& 			getAllHeaders() const;
};
	//std::string _bodyReceived;// Raw HTTP request string
