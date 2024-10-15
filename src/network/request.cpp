#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include "../include/HttpRequest.hpp"
#include "../include/CGI.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>

std::string getStatusMessage(int statusCode){ //Do I need to add more??????
	std::map<int, std::string> message = {
		{200, "OK"},
		{204, "No Content"},//The request has been successfully processed, but is not returning any content
		{400, "Bad Request"}, //The request cannot be fulfilled due to bad syntax
		{401, "Unauthorized"}, //The request was a legal request, but the server is refusing to respond to it. For use when authentication is possible but has failed or not yet been provided
		{404, "Not found"},//The requested page could not be found but may be available again in the future
		{405, "Method Not Allowed"},//A request was made of a page using a request method not supported by that page
		{408, "Request Timeout"}, //The server timed out waiting for the request 
		{413, "Request Too Large"},//The server will not accept the request, because the request entity is too large 
		{500, "Internal Server Error"}, //A generic error message, given when no more specific message is suitable
		{502, "Bad Gateway"},//The server was acting as a gateway or proxy and received an invalid response from the upstream server
		{504, "Gateway Timeout"},//The server was acting as a gateway or proxy and did not receive a timely response from the upstream server
	};
	if (message.count(statusCode))
		return message[statusCode];
	return "Unknown status";
}

void serveFile(int clientSocket, std::string filepath, int statusCode){
	std::ostringstream respond;
	std::ifstream file(filepath);//?
	std::cout << "Path: " << filepath << std::endl;
	if (!file){
		std::cout << "Wrong filepath" << std::endl;
		statusCode = 400;
	}
	std::stringstream buffer;
    buffer << file.rdbuf();
	std::string statusMessage = getStatusMessage(statusCode);
	respond << "HTTP/1.1 " << 200 << " " << statusMessage << "\r\n";
	respond << "Content-Type: text/html\r\n\r\n";
	respond << buffer.str();
	std::string respondStr = respond.str();
    send(clientSocket, respondStr.c_str(), respondStr.size(), 0);
	//close(clientSocket); //should I?

}

/*
Request:
GET /index.html HTTP/1.0
Host: localhost

add 2 empty lines*/

int checkErrors(std::string method, std::string version){
	std::cout << "ERROR CHECK";
	if (method != "GET" && method != "POST" && method != "DELETE")
		return 405;
	if (version != "HTTP/1.1")
		return 400;
}

int Server::handleRequest(int clientSocket, std::string request){

	std::istringstream request_stream(request);
	std::string method, path, version, host; //?

	request_stream >> method >> path >> version; //parse header
	std::cout << "Request: " << method << " " << path << std::endl;
	std::cout << "Host: " << host << std::endl;
	HttpRequest(method, path, version);
	int status = checkErrors(method, version);
	if (method == "GET"){
		std::string filepath = "www" + path;
		if (path == "/")
			filepath = "www/index.html";
		std::cout << "ERROR CHECK - 2";		
		serveFile(clientSocket, filepath, status);
		if (path.rfind("/cgi-bin/", 0) == 0) {// Path starts with "/cgi-bin/" 
			CGI cgi;
			cgi.handle_cgi_request(clientSocket, path, cgi);
		}
		return 0;
	}
	else if (method == "POST"){
		return 0;
	}
	else if (method == "DELETE"){
		return 0;
	}
	else
		return 1;
		// parse header and body
}