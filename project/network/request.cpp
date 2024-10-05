#include "../Server.hpp"
#include "../Client.hpp"
#include "../HttpRequest.hpp"
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


void serveFile(int clientSocket, std::string filepath){
	std::ifstream file(filepath);//?
	if (!file)
		std::cout << "Wrong filepath" << std::endl;
	std::stringstream buffer;
    buffer << file.rdbuf();

    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html\r\n\r\n"; 
    response += buffer.str();

    send(clientSocket, response.c_str(), response.size(), 0);
	close(clientSocket);
}

int Server::handleRequest(int clientSocket, std::string request){
	std::istringstream request_stream(request);
	std::string method, path, version; 
	request_stream >> method >> path >> version;
	std::cout << "Request: " << method << " " << path << std::endl;
	HttpRequest(method, path, version);
	if (method == "GET"){
		std::string filepath = "www" + path;
		if (path == "/")
			filepath = "www/test.html";
		serveFile(clientSocket, filepath);
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
		//parse header and body
}