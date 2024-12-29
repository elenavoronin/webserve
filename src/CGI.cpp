#include "../include/CGI.hpp"

/**
 * @brief       Constructor for the CGI class.
 */
CGI::CGI(HttpRequest *request) {
    _cgiInput = request->getField("body"); //can we just put this as a variable
    _inputIndex = 0;

    if (!setupPipes()) 
        return;

    _pid = fork();
    if (_pid == -1) {
        throw std::runtime_error("Fork failed!");
        // TODO set http response error
        return;
    }
    else if (_pid == 0) {
        handleChildProcess(request);
    } else {
        std ::cout << "going to parent process" << std::endl;
        handleParentProcess();
    }
}

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
void CGI::parseQueryString(HttpRequest* request) {

    if (_method == "GET") {
        _path = request->getField("path");
        std::size_t startPos = _path.find("?");
        if (startPos != std::string::npos) {
            _queryParams = _path.substr(startPos + 1);
        } 
        else {
            _queryParams = "";
        }
        if (_path.empty()) {
            std::cerr << "Error: Path is empty. Cannot extract query string." << std::endl;
            return;
        }
        else {
            //std::cout << "Request path: " << _path << std::endl;
        }
        //std::cout << "Extracted query string: " << _queryParams << std::endl;  // Debug
        //std::cout << "Parsing query string in process with PID: " << getpid() << std::endl;

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
void CGI::initializeEnvVars(HttpRequest* request) {

    _method = request->getField("method");
    _envVars.push_back("REQUEST_METHOD=" + _method);

    if (_method == "GET") {
        parseQueryString(request);
        _envVars.push_back("QUERY_STRING=" + _queryParams);
    } else if (_method == "POST") {
        std::string contentLength = request->getField("Content-Length");
        if (!contentLength.empty()) {
            _envVars.push_back("CONTENT_LENGTH=" + contentLength);
            //needs body?
        }
        std::string body = request->getField("body");
        if (!body.empty()) {
            _envVars.push_back("BODY=" + body);
        }
    } else if (_method == "DELETE") {
        parseQueryString(request);
        _envVars.push_back("QUERY_STRING=" + _queryParams);
        //std::cout << "QUERY_STRING: " << _queryParams << std::endl;
    } else {
        std::cerr << "Unsupported HTTP method: " << _method << std::endl;
        return;  // Or send an HTTP 405 response
    }

    // Add other common environment variables, such as SCRIPT_NAME
    _envVars.push_back("SCRIPT_NAME=" + request->getField("script_name"));

    for (const auto& var : _envVars) {
        _env.push_back(const_cast<char*>(var.c_str()));
    }
    _env.push_back(nullptr);
}


/**
 * @brief       Executes the CGI script with the specified environment variables.
 * 
 * @param       server     The Server object that may provide additional context or configuration.
 * 
 * @details     Uses `execve` to run the CGI script (`hello.py`) with the environment variables set up in `_env`.
 *              Redirects `stdout` to `_fromCgiPipe[WRITE]` so the output can be read back by the parent process.
 *              This function is meant to be called in the child process created by `fork`.
 * 
 * @todo        - Complete server configuration for CGI execution.
 *              - Add error handling for `execve`.
 *              - Implement dynamic script path generation based on server configuration??
 *              - Use Server object to set up the environment variables when properly configured
 *              - Check if the script path should be dynamically generated based on the server configuration.
 */
void CGI::executeCgi() {
    // Remove query string from _path
    std::size_t queryPos = _path.find("?");
    if (queryPos != std::string::npos) {
        _path = _path.substr(0, queryPos);  // Extract only the script path
    }

	std::string cgiProgramString = "./www/html" + _path;
    const char* cgiProgram = cgiProgramString.c_str();

    //std::cout << "Executing CGI script: " << cgiProgram << std::endl;
    // const std::string &cgiPass = server._location.get_cgi_pass();
    const char* argv[] = {"/usr/bin/python3", cgiProgram, nullptr};

    // const char* cgi_program = "./www/html/cgi-bin/hello.py";
    // const char* argv[] = {"/usr/bin/python3", cgi_program, nullptr};

    // std::string cgi_pass = server.getCgiPass();
    // std::string cgi_path = server.getCgiPath();

    dup2(_fromCgiPipe[WRITE], STDOUT_FILENO);
    close(_fromCgiPipe[READ]);
    close(_fromCgiPipe[WRITE]);

    std::cerr << "Executing: " << argv[0] << " " << argv[1] << std::endl;
    execve(argv[0], const_cast<char* const*>(argv), _env.data());	
	perror("execve failed"); // save status code somewhere

	exit(EXIT_FAILURE);
}

/**
 * @brief Reads the output from the CGI process via the pipe and sends it to the client.
 * 
 * @param client_socket The socket through which the server communicates with the client.
 * @todo                - Implement error handling if something goes wrong before sending the repsonse
 *                      - put buffer to 10 and have a read loop. after buffer is read return to poll and pass that 
 *                      - go back to the server and then continue reading
 *                      - handle 
 *                      - implement error handling 
 *                      - maybe use vector of characters to have less trouble with images
 * 
 */
void CGI::readCgiOutput() {
    char buffer[READ_SIZE];

    ssize_t bytes_read = read(_fromCgiPipe[READ], buffer, sizeof(buffer));
    if (bytes_read < 0) {
        throw std::runtime_error("Error reading from pipe");
    }
    if (bytes_read == 0) {
        markCgiComplete(); // EOF
        std::cerr << "Finished reading CGI output. Marking as complete." << std::endl;
        return;
    }
    else {
        _cgiOutput.append(buffer, bytes_read);
        std::cerr << "Read " << bytes_read << " bytes from CGI output." << std::endl;
    }
    
}

/**
 * @brief Writes the input data from the client to the CGI process via the pipe.
 *
 * This function writes chunks of the input data to the CGI process. The amount of data to write is limited by the
 * `WRITE_SIZE` constant. If the remaining data is less than `WRITE_SIZE`, only the remaining bytes are written.
 * The function keeps track of how much data has been written via the `_inputIndex` variable.
 *
 * @todo        - Implement error handling if something goes wrong with the write
 */
void CGI::writeCgiInput() {
    if (_inputIndex >= _cgiInput.size()) {
        close(_toCgiPipe[WRITE]); // Signal EOF to the CGI process
        std::cerr << "Finished writing to CGI. Closed write pipe." << std::endl;
        return;
    }

    unsigned long bytesToWrite = WRITE_SIZE;
    unsigned long bytesWritten = 0;

    if (bytesToWrite > _cgiInput.size() - _inputIndex) {
        bytesToWrite = _cgiInput.size() - _inputIndex;
    }
    std::cerr << "Writing to CGI: " << _cgiInput.substr(_inputIndex, bytesToWrite) << std::endl;
    //data + offset inputindex 
    bytesWritten = write(_toCgiPipe[WRITE], _cgiInput.data() + _inputIndex, bytesToWrite);
    if (bytesWritten < 0) {
        perror("Error writing to CGI pipe");
        throw std::runtime_error("Failed to write to CGI process");
    }
    _cgiInput += bytesWritten;
}

/**
 * @brief Sends the HTTP response to the client, using the CGI output as the body.
 * 
 * @param client_socket The socket to write the response to.
 * @param cgi_output The content generated by the CGI script.
 * 
 * @todo use buildResponse() ??
 * @todo check if output includes headers
 */
void CGI::sendResponse(int client_socket, const std::string& cgi_output) {
    HttpResponse response;
    // response.buildResponse();
    response.setStatus(200, "OK");
    response.setHeader("Content-Type", "text/html");
    response.setBody(cgi_output);

    std::string response_str = response.getFullResponse();
    ssize_t bytes_written = write(client_socket, response_str.c_str(), response_str.size());
    if (bytes_written == -1) {
        std::cerr << "Error: failed to write response to client socket" << std::endl;
    }
    close(client_socket);
}

/**
 * @brief Sets up the pipes for interprocess communication between the CGI process and the server.

 * @return true if the pipe setup was successful, false otherwise.
 * @todo change perror to throw
 */
bool CGI::setupPipes() {

    if (pipe(_fromCgiPipe) == -1) {
        perror("pipe failed");
        return false;
    }
    if (pipe(_toCgiPipe) == -1) {
        perror("pipe failed");
        close(_fromCgiPipe[READ]);
        close(_fromCgiPipe[WRITE]);
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
void CGI::handleChildProcess(HttpRequest* request) {
    close(_fromCgiPipe[READ]);
    initializeEnvVars(request);
    executeCgi();
}


/**
 * @brief Handles CGI processing in the parent process by closing the write end of the pipe to the CGI process and the read end of the pipe from the CGI process.
 * 
 * This method is called in the parent process after the child process has been forked. It closes the write end of the pipe to the CGI process and the read end of the pipe from the CGI process. This is necessary to prevent the parent process from writing to the pipe and to prevent the parent process from reading from the pipe.
 */
void CGI::handleParentProcess() {
    close(_fromCgiPipe[WRITE]);
    close(_toCgiPipe[READ]);
    //set everthing in HTTPresponse object
}


/**
 * @brief Gets the file descriptor for the read end of the pipe used for communication with the CGI process.
 * 
 * @return The file descriptor for the read end of the pipe.
 */
int CGI::getReadFd() const {
    return _fromCgiPipe[READ];
}

/**
 * @brief Gets the file descriptor for the write end of the pipe used for communication with the CGI process.
 * 
 * @return The file descriptor for the write end of the pipe.
 */
int CGI::getWriteFd() const {
    return _toCgiPipe[WRITE];
}

const std::string& CGI::getCgiOutput() const {
        return _cgiOutput;
}

void CGI::clearCgiOutput() {
        _cgiOutput.clear();
}

bool CGI::areHeadersSent() const {
        return _headersSent;
}

void CGI::markHeadersSent() {
        _headersSent = true;
}

bool CGI::isCgiComplete() const {
        return _cgiComplete;
}

void CGI::markCgiComplete() {
        _cgiComplete = true;
}

pid_t CGI::getPid() const {
    return _pid;
}
