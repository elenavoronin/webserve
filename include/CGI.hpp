#pragma once

#include <iostream>
#include <map>

// Reading environment variables like QUERY_STRING, REQUEST_METHOD, etc.
// Reading input from standard input (useful for POST requests).
// Generating HTTP headers (e.g., Content-Type, etc.).
// Sending the response back to the client.

class CGI {

	private:
		// Internal storage for CGI parameters (like parsed query strings)
		std::map<std::string, std::string> 	_query_params;
		// Internal storage for input data (POST request body)
		std::string 						_input_data;
		// Internal method to decode URL-encoded strings
		std::string urlDecode(const std::string& str);
		
	public:
		CGI();
		~CGI();

		// Method to read input (e.g., from POST requests)
		void read_input();
		// Method to get a specific environment variable
		std::string get_env(const std::string& var_name);
		// Method to parse the query string (for GET requests)
		void parse_query_string();
		// Method to get the value of a specific query parameter
		std::string get_query_param(const std::string& param_name);
		// Method to send the HTTP content-type header (like "Content-Type: text/html")
		void send_header(const std::string& content_type);
		// Method to send a complete HTTP response (status, headers, body)
		void send_response(const std::string& status, const std::map<std::string, std::string>& headers, const std::string& body);
		// Method to output HTML or other content
		void output(const std::string& content);

};