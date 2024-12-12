#pragma once

#include <string>
#include <map>

class HttpResponse {

	private:
		std::string							_fullResponse;
		int 								_statusCode;                                   // HTTP status code (e.g., 200, 404)
		std::string 						_statusMessage;                       // Corresponding status message (e.g., "OK", "Not Found")
		std::map<std::string, std::string> 	_headers;      		// HTTP headers (key-value pairs)
		std::string 						_body;                                // Response body content

	public:
		HttpResponse();                                   // Constructor to initialize default response
		~HttpResponse();                                  // Destructor

		// Setters for status, headers, and body
		void setStatus(int code, const std::string& message);   // Set HTTP status code and message
		void setHeader(const std::string& key, const std::string& value); // Add/modify a header
		void setBody(const std::string& content);               // Set the response body

		// Getters for status, headers, and body
		std::string getHeader(const std::string& key) const;    // Retrieve a header value
		std::string getStatusMessage() const;                   // Get the status message
		int getStatusCode() const;                              // Get the status code
		std::string &getFullResponse();

		// Build and manage response
		std::string buildResponse();                      // Construct full HTTP response
		void redirect(const std::string& location, int status_code, const std::string& message); // Set up redirection
};
