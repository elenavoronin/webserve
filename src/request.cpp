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
	// std::cout << "Path: " << filepath << std::endl;
	if (!file){
		std::cout << "Wrong filepath" << std::endl;
		statusCode = 400;
	}
	std::stringstream buffer;
    buffer << file.rdbuf(); //read file by bytes, go back to poll, check if finished reading
	//make 1 gb files to test
	std::string statusMessage = getStatusMessage(statusCode);
	respond << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
/*
For persistent connections (such as in HTTP/1.1), you would leave the client in the pfds list to handle further requests.
For non-persistent connections (such as in HTTP/1.0), it's appropriate to remove the client after processing the request.
*/
	respond << "Content-Type: text/html\r\n\r\n";
	respond << buffer.str();
	std::string respondStr = respond.str();
    send(clientSocket, respondStr.c_str(), respondStr.size(), 0);
	close(clientSocket); //should I?

}

/*
Request:
GET /index.html HTTP/1.0
Host: localhost

add 2 empty lines*/

// int checkErrors(std::string method, std::string version){
// 	std::cout << "ERROR CHECK";
// 	if (method != "GET" && method != "POST" && method != "DELETE")
// 		return 405;
// 	if (version != "HTTP/1.1")
// 		return 400;
	
// }

int checkErrors(std::string method, std::string version) {
    std::cout << "Checking for errors. Method: " << method << ", Version: " << version << std::endl;

    if (method != "GET" && method != "POST" && method != "DELETE") {
        std::cerr << "Error: Invalid method." << std::endl;
        return 405;
    }

    if (version.empty()) {
        std::cerr << "Error: Version is empty or invalid!" << std::endl;
        return 400;
    }

    if (version != "HTTP/1.1") {
        std::cerr << "Error: Invalid HTTP version." << std::endl;
        return 400;
    }

    return 200;
}

int HttpRequest::findContentLength(std::string request){
	this->readRequest(request);
	std::string len = this->getField("Content-length");
	std::cout << "Content length is " << len << std::endl;
	if (len != "")
		return(std::stoi(len));
	else
		return 0;
}


int Server::handleRequest(int clientSocket, std::string request, HttpRequest *Http){
/*
	TEST
			std::ifstream file("www/html/index.html");//?
			std::stringstream buffer;
			buffer << file.rdbuf();
			std::ostringstream respond;
			respond << "HTTP/1.1 " << 200 << " " << "OK" << "\r\n";
			respond << "Content-Type: text/html\r\n\r\n";
			respond << buffer.str();
			std::string respondStr = respond.str();
			send(clientSocket, respondStr.c_str(), respondStr.size(), 0);
			close(clientSocket);
			return 0;
*/
	std::istringstream request_stream(request);
	std::cout << "Request: " << request << std::endl;
	std::string method, path, version, host; //?
	request_stream >> method >> path >> version; //parse header
	Http->readRequest(request);
	// std::cout << "Request: " << method << " " << path << std::endl;
	// // HttpRequest(method, path, version);
	int status = Http->checkErrors();
	if (Http->getField("method") == "GET"){
		std::string filepath = "www/html" + Http->getField("path") ;
		if (Http->getField("path")  == "/")
			filepath = "www/html/index.html";	
		if (Http->getField("method").rfind("/cgi-bin/", 0) == 0) {// Path starts with "/cgi-bin/" 
			CGI cgi;
			cgi.handleCgiRequest(clientSocket, Http->getField("method") , *this);
		}
		serveFile(clientSocket, filepath, status);
		return 0;
	}
	// else if (path.rfind("/cgi-bin/", 0) == 0) {// Path starts with "/cgi-bin/"
	// 	CGI cgi;
	// 	std::string filepath = "www/" + path;
	// 	cgi.handleCgiRequest(clientSocket, filepath, *this);
	// 	// serveFile(clientSocket, filepath, status);
	// 	return 0;
	// }
	else if (Http->getField("method")  == "POST"){
		return 0;
	}
	else if (Http->getField("method")  == "DELETE"){
		return 0;
	}
	else
		return 1;
	// 	// parse header and body
	return 0;
}


/*
Sort stuff, add _port to private attributes
Rename methods with CamelCase
Check arommers
*/

/*
*/