#include "../include/CGI.hpp"

/**
 * @brief       Constructor for the CGI class.
 */
CGI::CGI(HttpRequest *request) {
    _cgiInput = request->getField("body"); // May be empty for GET requests
    _inputIndex = 0;
    _cgiComplete = false;
    _headersSent = false;

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
CGI::~CGI(){
    std::cout << "destructor called" << std::endl;
    close(_toCgiPipe[WRITE]);
    close(_toCgiPipe[READ]);
    close(_fromCgiPipe[WRITE]);
    close(_fromCgiPipe[READ]); 
}

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
        } else {
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
        }
        std::string body = request->getField("body");
        if (!body.empty()) {
            _envVars.push_back("BODY=" + body);
        }
    }

    // Additional standard CGI variables
    _envVars.push_back("SCRIPT_NAME=" + request->getField("script_name"));
    _envVars.push_back("SERVER_PROTOCOL=HTTP/1.1");
    _envVars.push_back("GATEWAY_INTERFACE=CGI/1.1");

    for (const auto& var : _envVars) {
        _env.push_back(const_cast<char*>(var.c_str()));
    }
    _env.push_back(nullptr); // End the environment variable list
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
 * @todo        
 *              - Add error handling for `execve`.
 *              - Implement dynamic script path generation based on server configuration??
 *              - Use Server object to set up the environment variables when properly configured
 *              - Check if the script path should be dynamically generated based on the server configuration.
 *              - Todo save status code somewhere
 */
void CGI::executeCgi() {
    std::size_t queryPos = _path.find("?");
    if (queryPos != std::string::npos) {
        _path = _path.substr(0, queryPos);
    }

    std::string scriptPath = "www/html" + _path;
    const char* cgiProgram = scriptPath.c_str();
    const char* argv[] = {"/usr/bin/python3", cgiProgram, nullptr};

    dup2(_fromCgiPipe[WRITE], STDOUT_FILENO);
    close(_fromCgiPipe[READ]);
    close(_fromCgiPipe[WRITE]);
    execve(argv[0], const_cast<char* const*>(argv), _env.data());	
	perror("execve failed");
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
 *                      - remove comments
 */
void CGI::readCgiOutput() {
    char buffer[READ_SIZE];

    ssize_t bytes_read = read(_fromCgiPipe[READ], buffer, sizeof(buffer));
    if (bytes_read < 0) {
        throw std::runtime_error("Error reading from pipe");
    }
    else if (bytes_read == 0) {
        std::cout << "EOF reached mark complete" << std::endl;
        markCgiComplete();
        std::cerr << "Finished reading CGI output. Marking as complete." << _cgiComplete << std::endl;
        return;
    }
    // Append data to output
    _cgiOutput.append(buffer, bytes_read);
    std::cerr << "Read " << bytes_read << " bytes from CGI output. Total output size: " 
              << _cgiOutput.size() << " bytes." << std::endl;
            //   << "Current CGI output: " << _cgiOutput << std::endl;
              
    // Parse headers if not sent
    if (!_headersSent) {

        auto headers_end = _cgiOutput.find("\r\n\r\n");
        if (headers_end == std::string::npos) {
            std::cerr << "Headers not yet complete. Waiting for more data." << std::endl;
            return; // Wait for more data in the next read
        }
        else if (headers_end != std::string::npos) {
            _headersSent = true;

            // Extract headers
            std::string headers = _cgiOutput.substr(0, headers_end);
            // std::cout << headers << std::endl;
            parseHeaders(headers);
            _receivedBodySize = _cgiOutput.size() - headers.size() - 5;


            std::cerr << "Headers received and parsed. Content-Length: " 
                << _contentLength << std::endl;
        }
        else {
            _receivedBodySize += bytes_read;

        }
    }
    _receivedBodySize += bytes_read;
    if (_receivedBodySize >= _contentLength) {
    std::cerr << "Body size matches Content-Length. Marking as complete." << std::endl;
    markCgiComplete();
    std::cerr << "Body fully received. Marking CGI as complete." << _cgiComplete << std::endl;
    }
}

/**
 * @brief       Parses HTTP headers to extract the Content-Length.
 * 
 * @param       headers     A string containing the HTTP headers to be parsed.
 * 
 * @details     This function reads through the given HTTP headers line by line,
 *              searching for the "Content-Length" header. If found, it extracts
 *              the value, converts it to an integer, and assigns it to `_contentLength`.
 *              If the "Content-Length" header is missing or malformed, it throws
 *              a `std::runtime_error`.
 * 
 * @throws      std::runtime_error if "Content-Length" header is missing or invalid.
 */
void CGI::parseHeaders(const std::string& headers) {
    std::istringstream headerStream(headers);
    std::string line;
    while (std::getline(headerStream, line)) {
        // std::cout << "line contains: " << line << std::endl;
        if (line.back() == '\r') {
            line.pop_back();
        }
        if (line.find("Content-Length:") == 0) {
            std::string contentLengthStr = line.substr(15);
        try {
            _contentLength = std::stoi(contentLengthStr);
            std::cerr << "Content-Length extracted: " << _contentLength << std::endl;
        } 
        catch (const std::exception& e) {
            std::cerr << "Invalid Content-Length header: " << contentLengthStr << std::endl;
            throw std::runtime_error("Invalid Content-Length value");
        }
        return;
        }
    }
    std::cerr << "Content-Length header not found in headers." << std::endl;
    throw std::runtime_error("Missing Content-Length header");
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
    
    std::cerr << "Writing to CGI: " << bytesToWrite << " bytes at index " << _inputIndex 
          << " (total size: " << _cgiInput.size() << ")" << std::endl;

    std::cout << "----------Writing to CGI---------" << std::endl;
    std::cout << " INPUT index is : " << _inputIndex << std::endl;

    if (bytesToWrite > _cgiInput.size() - _inputIndex) {
        bytesToWrite = _cgiInput.size() - _inputIndex;
    }
    std::cerr << "Writing to CGI: " << _cgiInput.substr(_inputIndex, bytesToWrite) << std::endl;
    bytesWritten = write(_toCgiPipe[WRITE], _cgiInput.data() + _inputIndex, bytesToWrite);
    if (bytesWritten <= 0) { //TODO omparison of unsigned expression < 0 is always false so set to <= instead of 0 to compile
        perror("Error writing to CGI pipe");
        throw std::runtime_error("Failed to write to CGI process");
    }
    _inputIndex += bytesWritten;
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

void CGI::setPath(std::string path){
    _path = path;
}

std::string CGI::getPath() const {
    return _path;
}
