#include "../include/CGI.hpp"

/**
 * @brief       Constructor for the CGI class.
 */
CGI::CGI(){}

/**
 * @brief       Destructor for the CGI class.
 */
CGI::~CGI(){}

/**
 * @brief       Parses the query string from the HTTP request path if the request method is GET.
 * 
 * @param       request    The HttpRequest object containing details of the HTTP request.
 * 
 * @details     If the request method is GET, this function extracts the query string 
 *              from the request path and stores it in `_queryParams`.
 * 
 * @todo        - Ensure `_method` is properly set before calling this function.
 *              - Add error handling in case the query string is malformed.
 */
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

/**
 * @brief       Initializes environment variables required for the CGI script.
 * 
 * @param       request    The HttpRequest object containing HTTP headers, method, and other details.
 * 
 * @details     Sets up key CGI environment variables like REQUEST_METHOD, QUERY_STRING, CONTENT_TYPE, 
 *              SCRIPT_NAME, and CONTENT_LENGTH. Converts these into a format that can be passed to `execve`.
 * 
 * @todo        - Verify if other environment variables are needed.
 *              - Handle cases where environment variables might be missing or malformed.
 *              - Ensure proper memory management for `_env` in case of re-initialization.
 */
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

    // Convert _envVars to char* format for execve
    for (const auto& var : _envVars) {
        _env.push_back(const_cast<char*>(var.c_str()));     // Convert strings to char* for execve
    }
    _env.push_back(nullptr);                                // Null-terminate for execve
}

/**
 * @brief       Executes the CGI script with the specified environment variables.
 * 
 * @param       server     The Server object that may provide additional context or configuration.
 * 
 * @details     Uses `execve` to run the CGI script (`hello.py`) with the environment variables set up in `_env`.
 *              Redirects `stdout` to `_responsePipe[WRITE]` so the output can be read back by the parent process.
 *              This function is meant to be called in the child process created by `fork`.
 * 
 * @todo        - Complete server configuration for CGI execution.
 *              - Add error handling for `execve`.
 *              - Implement dynamic script path generation based on server configuration??
 *              - Use Server object to set up the environment variables when properly configured
 *              - Check if the script path should be dynamically generated based on the server configuration.
 */
void CGI::executeCgi(Server server) {

    (void)server;   // TODO uncomment

	const char* cgi_program = "./www/html/cgi-bin/hello.py";
    const char* argv[] = {"/usr/bin/python3", cgi_program, nullptr};

    // std::string cgi_pass = server.getCgiPass();
    // std::string cgi_path = server.getCgiPath();

    // Redirect stdout to the write end of the pipe (to send CGI output back to parent)
    dup2(_responsePipe[WRITE], STDOUT_FILENO);
    close(_responsePipe[READ]);  // Close unused read end
    close(_responsePipe[WRITE]); // Close write end after dup2

    execve(argv[0], const_cast<char* const*>(argv), _env.data());	
	perror("execve failed"); // save status code somewhere

	exit(EXIT_FAILURE);
}

/**
 * @brief Reads the output from the CGI process via the pipe and sends it to the client.
 * 
 * @param client_socket The socket through which the server communicates with the client.
 * @todo                - Implement error handling if something goes wrong before sending the repsonse
 */
void CGI::readCgiOutput(int client_socket) {
    char buffer[1024];
    ssize_t bytes_read;

    while ((bytes_read = read(_responsePipe[READ], buffer, sizeof(buffer))) > 0) {
        if (bytes_read == -1) {
            std::cerr << "Error: read from pipe failed" << std::endl;
            return;
        }
        _cgiOutput.append(buffer, bytes_read);
    }

    close(_responsePipe[READ]);  // Close read end after finishing
    sendResponse(client_socket, _cgiOutput);
}

/**
 * @brief Sends the HTTP response to the client, using the CGI output as the body.
 * 
 * @param client_socket The socket to write the response to.
 * @param cgi_output The content generated by the CGI script.
 */
void CGI::sendResponse(int client_socket, const std::string& cgi_output) {
    HttpResponse response;
    response.setStatus(200, "OK");
    response.setHeader("Content-Type", "text/html");
    response.setBody(cgi_output);

    std::string response_str = response.buildResponse();
    ssize_t bytes_written = write(client_socket, response_str.c_str(), response_str.size());
    if (bytes_written == -1) {
        std::cerr << "Error: failed to write response to client socket" << std::endl;
    }
    close(client_socket);
}

/**
 * @brief Sets up the pipes for interprocess communication between the CGI process and the server.

 * @return true if the pipe setup was successful, false otherwise.
 */
bool CGI::setupPipes() {

    if (pipe(_responsePipe) == -1) {
        perror("pipe failed");
        return false;
    }
    return true;
}

/**
 * @brief Handles CGI processing in the child process by setting up environment variables
 *        and executing the CGI script.
 * 
 * @param request The HttpRequest object containing HTTP request details.
 * @param server The Server object that might provide configuration details.
 */
void CGI::handleChildProcess(HttpRequest& request, Server server) {
    close(_responsePipe[READ]);  // Close unused read end
    initializeEnvVars(request);  // Set up environment variables for CGI
    executeCgi(server);          // Run CGI script
}

/**
 * @brief Handles the parent process tasks, including waiting for the child process
 *        to finish and reading the CGI output.
 * 
 * @param client_socket The socket to write the response to the client.
 * @todo                - Implement reading from the pipe in chunks
 */
void CGI::handleParentProcess(int client_socket) {
    close(_responsePipe[WRITE]);  // Close unused write end

    // Wait for child process to finish and check for errors
    int status;
    waitpid(_pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        std::cerr << "Child process exited with error status " << WEXITSTATUS(status) << std::endl;
        //link error status it to http response status
        return;
    }
    readCgiOutput(client_socket);
}

/**
 * @brief Main function to handle the CGI request.
 * 
 * @param client_socket The socket through which the server communicates with the client.
 * @param path The path to the CGI script.
 * @param server The Server object that provides server configuration and utilities.
 * @param request The HttpRequest object containing HTTP request details (headers, body, etc.).
 */
void CGI::handleCgiRequest(int client_socket, const std::string& path, Server server, HttpRequest& request) {
    
    (void)path;
    if (!setupPipes()) return;

    _pid = fork();
    if (_pid == -1) {
        std::cerr << "Fork failed!" << std::endl;
        close(client_socket);
        return;
    }
    else if (_pid == 0) {
        handleChildProcess(request, server);
    } else {
        handleParentProcess(client_socket);
    }
}
