#pragma once

#include <iostream>
#include <map>
#include <string>
#include <cstdlib> //for getenv
#include <sstream> //for string stream manipulation
#include <unistd.h>
#include <sys/wait.h>

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


//Change to camelCase!

class CGI {

	private:
		// Example: Storing key environment variables //should I make this const?
    	const char* 						_env_vars[5] = {
        "REQUEST_METHOD=GET",
        "QUERY_STRING=name=Djoyke&age=33",
        "CONTENT_TYPE=text/html",
        "SCRIPT_NAME=/cgi-bin/hello.py",
        nullptr  // Null-terminated array
   		};
		// Internal storage for CGI parameters (like parsed query strings)
		std::map<std::string, std::string> 	_query_params;
		// Internal storage for input data (POST request body)
		std::string 						_input_data;
		std::string							_path;
		pid_t								_pid;
		int									_responsePipe[2];
		int									_requestPipe[2];
		// Internal method to decode URL-encoded strings
		std::string _urlDecode(const std::string& str);
		
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
		// Method to run the CGI script with environment variables
    	void execute_cgi();

		void handle_cgi_request(int client_socket, const std::string& path, CGI cgi);

};