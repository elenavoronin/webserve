#pragma once

#include <iostream>
#include <map>
#include <string>
#include <cstdlib>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include "Server.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

# define READ 0
# define WRITE 1

class CGI {

	private:
		std::vector<std::string> _envVars;          // Store environment variables as strings
		std::vector<char *> _env;                   // Convert to char* format for execve
		std::string _queryParams;                   // Store query parameters from the request
		std::string _inputData;                     // Store input data for POST requests
		std::string _method;                        // Store the HTTP request method (e.g., GET, POST)
		std::string _path;                          // Store the request path or script location
		std::string _pass;                          // Store the path to the CGI script
		pid_t _pid;                                 // Store the process ID for the CGI script
		int _status;                                // Store the exit status of the CGI script
		std::string _cgiOutput;                     // Store the output generated by the CGI script
		int _responsePipe[2];                       // Pipe for the parent process to read CGI responses
		int _requestPipe[2];                        // Pipe for the parent process to send data to CGI

		// Internal method to decode URL-encoded strings
		std::string _urlDecode(const std::string& str); // Decode URL-encoded strings

	public:
		CGI();                                      // Constructor
		~CGI();                                     // Destructor

		// Methods for managing input and communication
		bool setupPipes();                          // Set up pipes for communication with CGI
		void handleChildProcess(HttpRequest& request, Server server); // Handle CGI process logic for the child
		void handleParentProcess(int client_socket); // Handle parent process communication with the CGI

		// Methods for environment variable management
		void initializeEnvVars(HttpRequest& request); // Set up environment variables for the CGI script
		std::string getEnv(const std::string& var_name); // Retrieve a specific environment variable

		// Methods for parsing and managing query strings
		void parseQueryString(HttpRequest& request);   // Parse query string from the request
		std::string getQueryParam(const std::string& param_name); // Retrieve a specific query parameter

		// Methods for handling and sending responses
		void readCgiOutput(int client_socket);        // Read CGI output from the response pipe
		void sendResponse(int client_socket, const std::string& cgi_output); // Send CGI output as HTTP response
		void sendResponse(const std::string& status, const std::map<std::string, std::string>& headers, const std::string& body); // Send full HTTP response

		// Methods for HTTP headers and output
		void sendHeader(const std::string& content_type); // Send HTTP Content-Type header
		void output(const std::string& content);         // Output content (e.g., HTML)

		// Methods to handle and execute CGI scripts
		void executeCgi(Server server);                 // Execute the CGI script
		void handleCgiRequest(int client_socket, const std::string& path, Server server, HttpRequest &request); // Process CGI request
};
