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
#include "utils.hpp"
#include "EventPoll.hpp"
#include <ctime>
#include <chrono>

class Server;
class HttpRequest;
class HttpResponse;
class EventPoll;

/**
 * @brief The CGI class handles execution of CGI scripts and manages communication
 *        between the server and the CGI script using pipes.
 */
class CGI {

	private:
		std::vector<std::string> 						_envVars;
		std::vector<char *> 							_env;				
		std::string 									_queryParams;
		std::string 									_inputData;
		std::string 									_method;
		std::string 									_path;
		std::string 									_pass;
		pid_t 											_pid;
		std::string 									_cgiOutput;
		std::string 									_cgiInput;
		std::string 									_cgiPath;
		unsigned long 									_inputIndex;
		int 											_fromCgiPipe[2];
		int 											_toCgiPipe[2];
		bool 											_headersSent;
    	bool 											_cgiComplete;
		unsigned long									_contentLength = 0;
		unsigned long									_receivedBodySize = 0;

	public:
		CGI(HttpRequest* request);
		~CGI();

		bool 				setupPipes();              
		void 				handleChildProcess(HttpRequest* request);
		void 				handleParentProcess();
		void 				initializeEnvVars(HttpRequest* request);
		void 				parseQueryString(HttpRequest* request);
		void 				readCgiOutput();
		void 				writeCgiInput();
		void 				executeCgi();
		bool				isCgiComplete() const;
		void				markCgiComplete();
		void				parseHeaders(const std::string& headers);

		int 				getReadFd() const;
		int 				getWriteFd() const;
		const std::string&  getCgiOutput() const;
		pid_t				getPid() const;

};
