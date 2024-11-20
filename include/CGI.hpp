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
		std::vector<std::string>			_envVars; 			//store environment variables as strings
		std::vector<char *> 				_env;				//convert to char* format for execve
		std::string							_queryParams;		//store query parameters
		std::string 						_inputData;			//store input data
		std::string							_method;			//store request method
		std::string							_path;				//store request path
		pid_t								_pid;				//store process id
		int									_status;			//store exit status
		std::string							_cgiOutput;			//store CGI output
		int									_responsePipe[2];	//store response pipe
		int									_requestPipe[2]; 	//store request pipe
		
		// Internal method to decode URL-encoded strings
		std::string _urlDecode(const std::string& str); //need this?
		
	public:
		CGI();
		~CGI();

		// Method to read input (e.g., from POST requests)
		void readInput();
		bool setupPipes();
		void handleChildProcess(HttpRequest& request, Server server);
		void handleParentProcess(int client_socket);
		void initializeEnvVars(HttpRequest& request);
		void readCgiOutput(int client_socket);
		void sendResponse(int client_socket, const std::string& cgi_output);

		// Method to get a specific environment variable
		std::string getEnv(const std::string& var_name);

		// Method to parse the query string (for GET requests)
		void parseQueryString(HttpRequest& request);

		// Method to get the value of a specific query parameter
		std::string getQueryParam(const std::string& param_name);

		// Method to send the HTTP content-type header (like "Content-Type: text/html")
		void sendHeader(const std::string& content_type);

		// Method to output HTML or other content
		void output(const std::string& content);

		// Method to run the CGI script with environment variables
    	void executeCgi(Server server);

		// // Method to send a complete HTTP response (status, headers, body)
		void sendResponse(const std::string& status, const std::map<std::string, std::string>& headers, const std::string& body);
		void handleCgiRequest(int client_socket, const std::string& path, Server server, HttpRequest &request);
};


// const char* 						_envVars[7] = {
// "REQUEST_METHOD=POST", 								//extract from map made from request
// "QUERY_STRING=name=Djoyke&age=33",					//need to check this from the request n CGI
// "CONTENT_TYPE=text/html",							//get from map
// "SCRIPT_NAME=/cgi-bin/hello.py",						//get from map
// "BODY"="whatever this may be"/json/img/ whatever 	//get from map
// "CONTENT_LEnGHT"= "whatever that may be" 			//get from map
// nullptr  // Null-terminated array
// };

// Example: Storing key environment variables //should I make this const?
// const char* 						_envVars[5] = { //make dynamic
// "REQUEST_METHOD=GET", 				//extract from map made from request
// "QUERY_STRING=name=Djoyke&age=33",	//need to check this from the request n CGI
// "CONTENT_TYPE=text/html",			//get from map
// "SCRIPT_NAME=/cgi-bin/hello.py",	//get from map
// nullptr  // Null-terminated array
// };