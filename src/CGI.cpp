#include "../include/CGI.hpp"

//constructor
CGI::CGI(){}

//destructor
CGI::~CGI(){}

// Method to read input (e.g., from POST requests)
void CGI::readInput(){
	//is POST, GET or DELETE?
    //create env etc 

    //POST eg testimonials or upload file 
    //path naar script met bestand in body
    //content body needs to be path to file to be uploaded + content 
    //if post feed missing info to cgi script

}

// std::string CGI::getEnv(const std::string& var_name){

// }

void CGI::parseQueryString(HttpRequest& request) {

    if (_method == "GET") {
        _path = request.getField("path");
        std::size_t startPos = _path.find("?");
        if (startPos != std::string::npos) {
            _queryParams = _path.substr(startPos + 1);  // Extract query string
        } 
        else {
            _queryParams = "";
        }
    }
}

void CGI::initializeEnvVars(HttpRequest& request) {
    
    // Add REQUEST_METHOD from HttpRequest
    _method = request.getField("method");
    _envVars.push_back("REQUEST_METHOD=" + _method);
    
    // Add QUERY_STRING from request path or headers
    parseQueryString(request);
    _envVars.push_back("QUERY_STRING=" + _queryParams);
    
    // Add CONTENT_TYPE from HttpRequest headers, if it exists
    std::string contentType = request.getField("Content-Type");
    if (!contentType.empty()) {
        _envVars.push_back("CONTENT_TYPE=" + contentType);
    }
    
    // add SCRIPT_NAME
    _envVars.push_back("SCRIPT_NAME=" + request.getField("script_name"));

    // add BODY
    _envVars.push_back("BODY=" + request.getField("body"));
    // std::cout << "BODY ISSSS: " << request.getField("body") << std::endl;

    // add CONTENT_LENGHT
    _envVars.push_back("CONTENT_LENGHT=" + request.getField("content_lenght"));
    // std::cout << "CONTENT_LENGHT: " << request.getField("content_lenght") << std::endl;

    // Convert envVars to char* format for execve
    for (const auto& var : _envVars) {
        _env.push_back(const_cast<char*>(var.c_str()));     // Convert strings to char* for execve
    }
    _env.push_back(nullptr);                                // Null-terminate for execve
}

//executable checkt argv[1] but have to pass the executable as first argument
void CGI::executeCgi(Server server) {

    (void)server;   // TODO uncomment

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

    execve(argv[0], const_cast<char* const*>(argv), _env.data());	
	perror("execve failed");

	exit(EXIT_FAILURE);
}

//Function to handle CGI requests
//check for timeouts
//for GET Method
//if POST method
//create both GET and POST
// 1) how to check if body is done
// 2) pass it as body to response to be used
// 3) (later need to add to poll struct) and read as fast another poll adds and read
// 4) if error in child keep status code
void CGI::handleCgiRequest(int client_socket, const std::string& path, Server server, HttpRequest &request) {

    // _path = "." + path;  // Assuming the cgi-bin folder is in the current directory
    (void)path;//TODO uncomment this

    // create pipe for interprocess comunnication
    if (pipe(_responsePipe) == -1) {
        perror("pipe failed");
        return ;
    } 
    this->_pid = fork();
    if (this->_pid == -1) {
        std::cerr << "Fork failed!" << std::endl;
        close(client_socket);
        return ;
    }
    else if (this->_pid == 0) {
        //child writes
        close(_responsePipe[READ]);
        initializeEnvVars(request);
        this->executeCgi(server);
    }
    else {
        //parent reads
        close(_responsePipe[WRITE]);
        // wait for the child process to finish
        waitpid(this->_pid, nullptr, 0);
        //if status EXIT blabla then status code is BLABLA 

        //read this in chunks
        char buffer[1024];
        ssize_t bytes_read;
        std::string cgi_output;
        while ((bytes_read = read(_responsePipe[READ], buffer, sizeof(buffer))) > 0) { //read needs to go through poll
            // Check if the read operation was successful
            if (bytes_read == -1) {
                std::cerr << "Error: read from pipe failed" << std::endl;
                return ;
            }
            cgi_output.append(buffer, bytes_read);
            close(_responsePipe[READ]);

            HttpResponse response;
            response.setStatus(200, "OK"); //will become obsolete
            response.setHeader("Content-Type", "text/html");  //will become obsolete, Sets appropriate content type
            response.setBody(cgi_output);  // Set the CGI output as the response body, send this to vector only

            //will become obsolete when poll vector is done
            // Send response to client, will become obsolete when poll vector is done, send to function client
            std::string response_str = response.buildResponse();
            ssize_t bytes_written = write(client_socket, response_str.c_str(), response_str.size());
            if (bytes_written == -1) {
                std::cerr << "Error: failed to write response to client socket" << std::endl;
            }
            write(client_socket, response_str.c_str(), response_str.size());
            close(client_socket);
        }
        // Check if the read operation failed
        if (bytes_read == -1) {
            std::cerr << "Error: read from pipe failed" << std::endl;
            return ;
        }
        // Close the pipe file descriptor
        if (close(_responsePipe[READ]) == -1) {
            std::cerr << "Error: unable to close pipe file descriptor" << std::endl;
            return ;
        }
        close(client_socket);
    }
}
