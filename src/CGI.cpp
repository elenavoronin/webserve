#include "../include/CGI.hpp"

//constructor
CGI::CGI(){
	std::cout<< "CGI constructor called" << std::endl;
}

//destructor
CGI::~CGI(){
	std::cout<< "CGI destructor called" << std::endl;
}

// Method to read input (e.g., from POST requests)
void::CGI::readInput(){
	
}

//executable checkt argv[1] but have to pass the executable as first argument
void CGI::executeCgi() {

	const char* cgi_program = "./www/html/cgi-bin/hello.py";
	const char* argv[] = {"/usr/bin/python3", cgi_program, nullptr};

	execve(argv[0], const_cast<char* const*>(argv), const_cast<char* const*>(_envVars));
	
	perror("execve failed");
	exit(EXIT_FAILURE);
}

// Function to handle CGI requests
void CGI::handleCgiRequest(int client_socket, const std::string& path, CGI cgi) {
    std::cout << "cgi here" << std::endl;
    // _path = "." + path;  // Assuming the cgi-bin folder is in the current directory

    pipe(_requestPipe); // create pipe for interprocess comunnication
    cgi._pid = fork();

    if (cgi._pid == -1) {
        std::cerr << "Fork failed!" << std::endl;
        close(client_socket);
        return ;
    }
    else if (cgi._pid == 0) {
        std::cout << "This is the child process with PID: " << getpid() << std::endl;

        //child writes

        close(_requestPipe[READ]);
        dup2(_requestPipe[WRITE], STDOUT_FILENO);
        close(_requestPipe[WRITE]);
        // choose environmental variable
        dup2(client_socket, STDOUT_FILENO); // is this legal
        close(client_socket);
        cgi.executeCgi();
    }
    else {
        std::cout << "This is the parent process. Child PID: " << cgi._pid << std::endl;

        //parent reads

        // wait for the child process to finish
        waitpid(cgi._pid, nullptr, 0);
        // while (waitpid(cgi._pid, nullptr, 0))
        //     //read from child pipe
        
        //parent writes to the client //send response to client
        close(client_socket);
    }
}