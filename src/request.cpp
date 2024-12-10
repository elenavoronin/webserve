#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include "../include/HttpRequest.hpp"
#include "../include/HttpResponse.hpp"
#include "../include/Location.hpp"
#include "../include/utils.hpp"
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



std::string readFileContent(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file) {
        std::cerr << "Error: File not found: " << filepath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf(); //read file by bytes, go back to poll, check if finished reading
	//request->_readyToSendBack = true;
    return buffer.str();
}

void sendHeaders(int clientSocket, int statusCode, const std::string& contentType = "text/html") {
    std::string statusMessage = getStatusMessage(statusCode);
    std::ostringstream headers;
    headers << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
// For persistent connections (such as in HTTP/1.1), you would leave the client in the pfds list to handle further requests.
// For non-persistent connections (such as in HTTP/1.0), it's appropriate to remove the client after processing the request.
    headers << "Content-Type: " << contentType << "\r\n\r\n"; // Default is text/html
    std::string headersStr = headers.str();
    send(clientSocket, headersStr.c_str(), headersStr.size(), 0);
}

// Function to send the body of the HTTP response
void sendBody(int clientSocket, const std::string& body) {
    send(clientSocket, body.c_str(), body.size(), 0);
}


int validateRequest(const std::string& method, const std::string& version) {
	if (method != "GET" && method != "POST" && method != "DELETE") {
		std::cerr << "Error: Unsupported HTTP method." << std::endl;
		return 405; // Method Not Allowed
	}

	if (version != "HTTP/1.1") {
		std::cerr << "Error: Invalid HTTP version. Only HTTP/1.1 is supported." << std::endl;
		return 400;
	}
	return 200;
}

void sendFileResponse(int clientSocket, const std::string& filepath, int statusCode) {
	std::string fileContent = readFileContent(filepath);
	if (fileContent.empty()) {
		sendHeaders(clientSocket, 404);
		sendBody(clientSocket, "<html><body>404 - File Not Found</body></html>");
	} else {
		sendHeaders(clientSocket, statusCode);
		sendBody(clientSocket, fileContent);
	}
	close(clientSocket);
}

int Server::handleGetRequest(int clientSocket, const std::string& path, HttpRequest* Http) {
	
	(void)Http;
	
	std::string filepath = this->getRoot() + '/' + path;
	if (path == "/") {
		filepath = this->getRoot() + '/' + this->getIndex();
	}
	if (path.rfind("/cgi-bin/", 0) == 0) { //change to config
		CGI cgi;
		cgi.handleCgiRequest(clientSocket, path, *this, *Http);
		return 0;
	}
	std::ifstream file(filepath);
	if (!file) {
		std::cerr << "Error: File not found for path " << filepath << std::endl;
		sendFileResponse(clientSocket, "www/html/404.html", 404);
		return 404;
	}
	sendFileResponse(clientSocket, filepath, 200);
	return 200;
}

int Server::handlePostRequest(int clientSocket, const std::string& path, HttpRequest* Http) {
	
	(void)clientSocket;
	(void)path;
	(void)Http;

	// if (!Http->findContentLength(Http->_strReceived) || Http->findContentLength(Http->_strReceived) <= 0) //Can be equal to 0?
	// 	return 400;
	// // if (Http->findContentLength(Http->_strReceived) > this->getMaxlength()) //add after parsing
	// // 	return 413;
	// if (Http->getField("Content-type") == "application/x-www-form-urlencoded")




/*Extract the HTTP headers to understand what type of data is being sent (e.g., Content-Type, Content-Length).
If the body size exceeds a predefined limit (for example, from a config file), return an error like 413 Request Too Large.
Ensure that the Content-Length header is present and valid. This will tell you how much data to expect in the body.
Read the body of the request from the socket. You might need to handle partial reads (i.e., the body could arrive in chunks).
Form Data: If Content-Type is application/x-www-form-urlencoded, parse the form fields and their values.
JSON Data: If the request contains application/json, you can parse the JSON data and extract the necessary information.
File Uploads: If the Content-Type is multipart/form-data, handle file uploads (you'll need to parse the file boundaries and save the file to disk).
Form Submission: Save data to a database, perform an action, or return a response to confirm the form was submitted.
File Upload: Save the file to a specific directory and generate a success/failure response.
Once processing is complete, send an appropriate response to the client. 
Success (200 OK): If the request was processed successfully.
Error (400 Bad Request): If there was a problem with the data.
Error (500 Internal Server Error): If something went wrong on the server side.
Decide whether to close the connection or keep it alive (based on HTTP version or a Connection header).
*/
	return 0;
}

int Server::handleDeleteRequest(int clientSocket, const std::string& path, HttpRequest* Http) {
/*
Extract the path from the request (usually the file or resource to be deleted).
Check if the requested resource exists. If not, return a 404 Not Found response.
For security reasons, it's essential to implement some form of authorization to ensure that only authorized users can delete resources.
If the user isn't authorized, return a 401 Unauthorized or 403 Forbidden.
Ensure the resource being deleted is a valid file, directory, or resource that can be deleted. For example, check if it’s a file in a specific directory on the server.
Optionally, check if the file can be deleted (i.e., it’s not locked or in use).
Perform the deletion. For files or directories, use appropriate system calls to delete the resource.
If it’s a database entry or another type of resource, ensure the record is properly deleted from the data store.
Check if there are any issues while deleting (e.g., file permissions, file not found, or resource is locked).
If an error occurs during the deletion process, return a 500 Internal Server Error or a more specific status code.
If the deletion was successful, send a 200 OK or 204 No Content response.
If there was an issue, return a corresponding error code:
403 Forbidden: If the user is not allowed to delete the resource.
404 Not Found: If the file or resource does not exist.
500 Internal Server Error: If there was an error during the deletion process.
It’s often useful to log the deletion operation for auditing purposes, especially if your server manages important data.
As with the POST request, decide whether to close the connection or keep it alive based on the HTTP version or the Connection header.
*/

	(void)clientSocket;
	(void)path;
	(void)Http;

	return 0;
}


int Server::processClientRequest(int clientSocket, const std::string& request, HttpRequest* HttpRequest) {
	std::istringstream requestStream(request);
	std::string method, path, version;
	HttpResponse response;
	requestStream >> method >> path >> version;
	HttpRequest->readRequest(request);
	checkLocations(path);
	int status = validateRequest(method, version);
	std::cout << status << std::endl;
	// std::cout << "Content-type: " << Http->getField("Content-type") << std::endl;
	if (status != 200) {
		sendFileResponse(clientSocket, "www/html/500.html", status);  //change to a config ones?
		return status;
	}
	if (method == "GET" && std::find(this->_allowed_methods.begin(), this->_allowed_methods.end(), "GET") != this->_allowed_methods.end())
		//check with this endpoint am I allowed to use get?
		return handleGetRequest(clientSocket, path, HttpRequest); //?? what locations should be passed
	if (method == "POST" && std::find(this->_allowed_methods.begin(), this->_allowed_methods.end(), "POST") != this->_allowed_methods.end())
	//check with this endpoint am I allowed to use post?
		return handlePostRequest(clientSocket, path, HttpRequest);
	if (method == "DELETE" && std::find(this->_allowed_methods.begin(), this->_allowed_methods.end(), "DELETE") != this->_allowed_methods.end())
	//check with this endpoint am I allowed to use delete?
		return handleDeleteRequest(clientSocket, path, HttpRequest);
	sendResponse(clientSocket, response.buildResponse());
	return 0;
}
