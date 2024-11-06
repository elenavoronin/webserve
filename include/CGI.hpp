#pragma once

#include <iostream>
#include <map>
#include <string>
#include <cstdlib> //for getenv
#include <sstream> //for string stream manipulation
#include <unistd.h>
#include <sys/wait.h>
#include "Server.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

# define READ 0
# define WRITE 1

/**
 * urlDecode: 			This method decodes URL-encoded strings, 
 * 						which is useful when processing query parameters 
 * 						from a GET request or form data.
 * read_input: 			This reads the input from stdin, 
 * 						typically for POST requests.
 * get_env: 			This retrieves environment variables 
 * 						like QUERY_STRING, REQUEST_METHOD, etc., 
 * 						that are made available by the web server.
 * parse_query_string: 	This parses the QUERY_STRING into key-value pairs 
 * 						and stores them in _query_params.
 * get_query_param: 	This retrieves a specific query parameter's value 
 * 						from the _query_params map.
 * send_header: 		This sends HTTP headers back to the client.
 * send_response: 		This constructs and sends a full HTTP response, 
 * 						including status, headers, and body.
 * output: 				This outputs the actual content to stdout, 
 * 						which will be served to the client.
 */
class CGI {

	private:
		// Example: Storing key environment variables //should I make this const?
    	const char* 						_envVars[5] = { //make dynamic
        "REQUEST_METHOD=GET", 				//extract from map made from request
        "QUERY_STRING=name=Djoyke&age=33",	//need to check this from the request n CGI
        "CONTENT_TYPE=text/html",			//get from map
        "SCRIPT_NAME=/cgi-bin/hello.py",	//get from map
        nullptr  // Null-terminated array
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

		// Internal storage for CGI parameters (like parsed query strings)
		std::map<std::string, std::string> 	_queryParams;
		// const HttpRequest& 					_hhtpRequest; //reference to existing request
		// Internal storage for input data (POST request body)
		std::string 						_inputData;
		std::string							_path;
		pid_t								_pid;
		int									_responsePipe[2];
		int									_requestPipe[2];
		// Internal method to decode URL-encoded strings
		std::string _urlDecode(const std::string& str);
		
	public:
		// CGI(const HttpRequest& httpRequest) : _hhtpRequest(httpRequest) {};
		CGI();
		~CGI();

		// Method to read input (e.g., from POST requests)
		void readInput();
		// Method to get a specific environment variable
		std::string getEnv(const std::string& var_name);
		// Method to parse the query string (for GET requests)
		void parseQueryString();
		// Method to get the value of a specific query parameter
		std::string getQueryParam(const std::string& param_name);
		// Method to send the HTTP content-type header (like "Content-Type: text/html")
		void sendHeader(const std::string& content_type);
		// // Method to send a complete HTTP response (status, headers, body)
		// void sendResponse(const std::string& status, const std::map<std::string, std::string>& headers, const std::string& body);
		// Method to output HTML or other content
		void output(const std::string& content);
		// Method to run the CGI script with environment variables
    	void executeCgi(Server server);

		void handleCgiRequest(int client_socket, const std::string& path, Server server);

};



// Request: GET /cgi-bin/hello.py?name=John&age=30 HTTP/1.1
// Host: localhost:8080
// Connection: keep-alive
// sec-ch-ua: "Google Chrome";v="129", "Not=A?Brand";v="8", "Chromium";v="129"
// sec-ch-ua-mobile: ?0
// sec-ch-ua-platform: "Linux"
// Upgrade-Insecure-Requests: 1
// User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/129.0.0.0 Safari/537.36
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
// Sec-Fetch-Site: none
// Sec-Fetch-Mode: navigate
// Sec-Fetch-User: ?1
// Sec-Fetch-Dest: document
// Accept-Encoding: gzip, deflate, br, zstd
// Accept-Language: en-US,en;q=0.9