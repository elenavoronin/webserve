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
	//is POST, GET or DELETE?
    //create env etc 

    //POST eg testimonials or upload file 
    //path naar script met bestand in body
    //content body needs to be path to file to be uploaded + content 
    //if post feed missing info to cgi script

}

//executable checkt argv[1] but have to pass the executable as first argument
void CGI::executeCgi(Server server) {

	const char* cgi_program = "./www/html/cgi-bin/hello.py";
    const char* argv[] = {"/usr/bin/python3", cgi_program, nullptr};

    // std::string cgi_pass = server.getCgiPass();
    // std::string cgi_path = server.getCgiPath();
    
    // const char* argv[] = {cgi_pass.c_str(), cgi_path.c_str(), nullptr};

	execve(argv[0], const_cast<char* const*>(argv), const_cast<char* const*>(_envVars));
	
	perror("execve failed");
	exit(EXIT_FAILURE);
}

// Function to handle CGI requests
void CGI::handleCgiRequest(int client_socket, const std::string& path, Server server) {
    // std::cout << "cgi here" << std::endl;
    // _path = "." + path;  // Assuming the cgi-bin folder is in the current directory

    //for GET Method
    pipe(_responsePipe); // create pipe for interprocess comunnication
    //if POST method
    //create both GET and POST
    this->_pid = fork();

    if (this->_pid == -1) {
        std::cerr << "Fork failed!" << std::endl;
        close(client_socket);
        return ;
    }
    else if (this->_pid == 0) {
        // std::cout << "This is the child process with PID: " << getpid() << std::endl;
        //child writes

        //als POST
        //dup request to STDIN

        close(_responsePipe[READ]);
        dup2(_responsePipe[WRITE], STDOUT_FILENO);
        close(_responsePipe[WRITE]);
        // choose environmental variable
        dup2(client_socket, STDOUT_FILENO); // is this legal
        close(client_socket);
        this->executeCgi(server);
    }
    else {
        // std::cout << "This is the parent process. Child PID: " << this->_pid << std::endl;
        //parent reads

        // wait for the child process to finish
        waitpid(this->_pid, nullptr, 0);
        // 1)read from child pipe
        // 2) pass it as body to response to be used
        // 3)(later need to add to poll struct) and read as fast asother poll adds and read
        // 4) if error in child keep status code


        // while (waitpid(this->_pid, nullptr, 0))
        //     //read from child pipe
        
        //parent writes to the client //send response to client
        close(client_socket);
    }
}