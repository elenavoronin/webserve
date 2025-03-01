#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "CGI.hpp"
#include <arpa/inet.h>
#include <netdb.h>
#include "utils.hpp"
#include "EventPoll.hpp"
#include "Server.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <chrono>
#include <ctime>

class HttpRequest;
class HttpResponse;
class CGI;
class Server;

struct defaultServer;



/**
 * @brief The Client class represents a single client connection to the server.
 *
 * This class manages the state and behavior of a client connection, including
 * its socket, associated HTTP request and response objects, and optional CGI handling.
 */
class Client {
	private:
		int 			_clientSocket;
		HttpRequest* 	_HttpRequest;
		HttpResponse* 	_HttpResponse;
		CGI*			_CGI;
		EventPoll*		_eventPoll;
		unsigned long	_responseIndex;
		std::chrono::steady_clock::time_point			_start_time;


	public:
		Client(int clientSocket, EventPoll& eventPoll);
		~Client();
		Client& 			operator=(const Client& copy);
		Client(const Client& copy);
		void 				setSocket(int clientSocket);
		int 				getSocket() const;
		HttpRequest* 		getHttpRequest() const;
		HttpResponse* 		getHttpResponse() const;
		void 				setHttpRequest(HttpRequest* httpRequest);
		void 				setHttpResponse(HttpResponse* httpResponse);
		int 				writeToSocket();
		void 				readFromSocket(Server *server, defaultServer defaultS, std::vector<defaultServer> servers);
		void 				closeConnection(EventPoll& eventPoll, int currentPollFd);
		void				addToEventPollRemove(int fd, int eventType);
		void 				addToEventPollQueue(int fd, int eventType);
		
		//CGI calling methods
		int 				getCgiRead();
		int 				getCgiWrite();
		void 				startCgi(HttpRequest *request);
		void 				readFromCgi();
		void 				writeToCgi();
		CGI*				getCGI() const;



		void 				setStartTime(std::chrono::steady_clock::time_point start_time);
		std::chrono::steady_clock::time_point  				getStartTime() const;
};
