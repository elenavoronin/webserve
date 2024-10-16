#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <cstring>
#include "../include/CGI.hpp"

// Port to run the server on

CGI cgi;

// Function to handle CGI requests
void handle_cgi_request(int client_socket, const std::string& path) {
    std::cout << "cgi here" << std::endl;

    // _path = "." + path;  // Assuming the cgi-bin folder is in the current directory

    int pipefd[2];
    pipe(pipefd); // create pipe for interprocess comunnication
    pid_t pid = fork();

    if (pid == -1) {
        std::cerr << "Fork failed!" << std::endl;
        close(client_socket);
        return ;
    }
    else if (pid == 0) {
        std::cout << "This is the child process with PID: " << getpid() << std::endl;

        //child writes

        // close(pipefd[READ]);
        // dup2(pipefd[WRITE], STDOUT_FILENO);
        // close(pipefd[WRITE]);
        // choose environmental variable
        dup2(client_socket, STDOUT_FILENO); // is this legal
        close(client_socket);
        cgi.execute_cgi();
    }
    else {
        std::cout << "This is the parent process. Child PID: " << pid << std::endl;

        //parent reads

        // wait for the child process to finish
        waitpid(pid, nullptr, 0);
        // while (waitpid(pid, nullptr, 0))
        //     //read from child pipe
        
        //parent writes to the client //send response to client
        close(client_socket);
    }

    // Example of running a simple CGI script (you can replace this with real logic)
    // std::string cgi_response = "<html><body><h1>CGI Script Response</h1><p>This is output from your CGI script.</p></body></html>";
    
    // std::string http_response = "HTTP/1.1 200 OK\r\n"
    //                             "Content-Type: text/html\r\n"
    //                             "Content-Length: " + std::to_string(cgi_response.length()) + "\r\n"
    //                             "Connection: close\r\n"
    //                             "\r\n" +
    //                             cgi_response;

    // send(client_socket, http_response.c_str(), http_response.size(), 0);
}

// // Function to handle non-CGI requests
// void handle_non_cgi_request(int client_socket, const std::string& path) {
//     // Simple example of serving an HTML file or a 404 page
//     std::ifstream file("html" + path);  // Look for files in the htdocs folder

//     std::cout << "non cgi here" << std::endl;

//     if (file.is_open()) {
//         std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        
//         std::string http_response = "HTTP/1.1 200 OK\r\n"
//                                     "Content-Type: text/html\r\n"
//                                     "Content-Length: " + std::to_string(content.length()) + "\r\n"
//                                     "Connection: close\r\n"
//                                     "\r\n" + content;
                                    
//         send(client_socket, http_response.c_str(), http_response.size(), 0);
//     } else {
//         std::string not_found = "<html><body><h1>404 Not Found</h1></body></html>";
//         std::string http_response = "HTTP/1.1 404 Not Found\r\n"
//                                     "Content-Type: text/html\r\n"
//                                     "Content-Length: " + std::to_string(not_found.length()) + "\r\n"
//                                     "Connection: close\r\n"
//                                     "\r\n" + not_found;
//         send(client_socket, http_response.c_str(), http_response.size(), 0);
//     }
// }

// // Function to handle client requests
// void handle_client(int client_socket) {
//     char buffer[1024] = {0};
//     int read_size = read(client_socket, buffer, 1024);
    
//     if (read_size > 0) {
//         // Parse the HTTP request
//         std::istringstream request_stream(buffer);
//         std::string request_line;
//         std::getline(request_stream, request_line);

//         // Simple HTTP request parsing
//         std::istringstream line_stream(request_line);
//         std::string method, path, http_version;
//         line_stream >> method >> path >> http_version;

//         std::cout << "Received request: " << method << " " << path << " " << http_version << std::endl;

//         // Check if the request is for a CGI script
//         if (path.rfind("/cgi-bin/", 0) == 0) {  // Path starts with "/cgi-bin/"
//             handle_cgi_request(client_socket, path);
//         } else {
//             handle_non_cgi_request(client_socket, path);
//         }
//     }

//     close(client_socket);
// }

// // Main server loop
// int main() {
//     int server_fd, client_socket;
//     struct sockaddr_in address;
//     int addrlen = sizeof(address);

//     // Create a socket
//     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
//         perror("Socket failed");
//         exit(EXIT_FAILURE);
//     }

//     // Set socket options
//     int opt = 1;
//     if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
//         perror("Setsockopt failed");
//         exit(EXIT_FAILURE);
//     }

//     // Bind the socket to the port
//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = INADDR_ANY;
//     address.sin_port = htons(PORT);
//     if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
//         perror("Bind failed");
//         exit(EXIT_FAILURE);
//     }

//     // Listen for incoming connections
//     if (listen(server_fd, 10) < 0) {
//         perror("Listen failed");
//         exit(EXIT_FAILURE);
//     }

//     std::cout << "Server is listening on port " << PORT << "..." << std::endl;

//     // Main loop: accept and handle client connections
//     while (true) {
//         if ((client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
//             perror("Accept failed");
//             exit(EXIT_FAILURE);
//         }

//         // Handle the client request in a new thread
//         std::thread(handle_client, client_socket).detach();
//     }

//     return 0;
// }
