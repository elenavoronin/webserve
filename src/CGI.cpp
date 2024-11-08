#include "../include/CGI.hpp"

//constructor
CGI::CGI(){}

//destructor
CGI::~CGI(){}

// Method to read input (e.g., from POST requests)
void::CGI::readInput(){
	//is POST, GET or DELETE?
    //create env etc 

    //POST eg testimonials or upload file 
    //path naar script met bestand in body
    //content body needs to be path to file to be uploaded + content 
    //if post feed missing info to cgi script

}

//executable checkt argv[1] but have to pass the executable as first argument
void CGI::executeCgi(Server server) {

    (void)server;//TODO uncomment

	const char* cgi_program = "./www/html/cgi-bin/hello.py";
    const char* argv[] = {"/usr/bin/python3", cgi_program, nullptr};

    // std::string cgi_pass = server.getCgiPass();
    // std::string cgi_path = server.getCgiPath();

    // Set up necessary environment variables?
    // setenv("REQUEST_METHOD", "GET", 1);  // Example, adjust as needed
    // setenv("QUERY_STRING", "name=John&age=30", 1);  // Example query string
    
    // const char* argv[] = {cgi_pass.c_str(), cgi_path.c_str(), nullptr};

    // Redirect stdout to the write end of the pipe (to send CGI output back to parent)
    dup2(_responsePipe[WRITE], STDOUT_FILENO);
    close(_responsePipe[READ]);  // Close unused read end
    close(_responsePipe[WRITE]); // Close write end after dup2

	execve(argv[0], const_cast<char* const*>(argv), const_cast<char* const*>(_envVars));
	
	perror("execve failed");
	exit(EXIT_FAILURE);
}

// Function to handle CGI requests
void CGI::handleCgiRequest(int client_socket, const std::string& path, Server server) {
    // _path = "." + path;  // Assuming the cgi-bin folder is in the current directory
    //for GET Method
    // create pipe for interprocess comunnication

    (void)path;//TODO uncomment this

    if (pipe(_responsePipe) == -1) {
        perror("pipe failed");
        return ;
    } 

    //if POST method
    //create both GET and POST
    this->_pid = fork();

    if (this->_pid == -1) {
        std::cerr << "Fork failed!" << std::endl;
        close(client_socket);
        return ;
    }
    else if (this->_pid == 0) {
        //child writes
        close(_responsePipe[READ]);
        this->executeCgi(server);
    }
    else {
        //parent reads
        close(_responsePipe[WRITE]);
        // wait for the child process to finish
        waitpid(this->_pid, nullptr, 0);

        //read this in chunks
        char buffer[1024];
        ssize_t bytes_read;
        std::string cgi_output;
        while ((bytes_read = read(_responsePipe[READ], buffer, sizeof(buffer))) > 0) {
            // Check if the read operation was successful
            if (bytes_read == -1) {
                std::cerr << "Error: read from pipe failed" << std::endl;
                return;
            }
            cgi_output.append(buffer, bytes_read);
            close(_responsePipe[READ]);

            HttpResponse response;
            response.setStatus(200, "OK");
            response.setHeader("Content-Type", "text/html");  // Set appropriate content type
            response.setBody(cgi_output);  // Set the CGI output as the response body

            // Send response to client
            std::string response_str = response.buildResponse();
            ssize_t bytes_written = write(client_socket, response_str.c_str(), response_str.size());
            if (bytes_written == -1) {
                std::cerr << "Error: failed to write response to client socket" << std::endl;
            }
            
            // // TODO Djoyke: append to map instead
            // // Send headers and the CGI response to the client
            // std::string response = "HTTP/1.1 200 OK\r\n";
            // response += "Content-Type: text/html\r\n";
            // response += "Content-Length: " + std::to_string(cgi_output.size()) + "\r\n";
            // response += "\r\n";
            // response += cgi_output;  // Append the CGI output as the response body

            write(client_socket, response_str.c_str(), response_str.size());
            close(client_socket);
        }
        // Check if the read operation failed
        if (bytes_read == -1) {
            std::cerr << "Error: read from pipe failed" << std::endl;
            return;
        }
        // Close the pipe file descriptor
        if (close(_responsePipe[READ]) == -1) {
            std::cerr << "Error: unable to close pipe file descriptor" << std::endl;
            return ;
        }
        // 2) pass it as body to response to be used
        // 3)(later need to add to poll struct) and read as fast asother poll adds and read
        // 4) if error in child keep status code
        close(client_socket);
    }
}