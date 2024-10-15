#include "../include/CGI.hpp"

//constructor
CGI::CGI(){
	std::cout<< "CGI constructor called" << std::endl;
}

CGI::~CGI(){
	std::cout<< "CGI destructor called" << std::endl;
}

// void::CGI::read_input(){
	
// }

//executable checkt argv[1] but have to pass the executable as first argument
void CGI::execute_cgi() {

	const char* cgi_program = "./www/html/cgi-bin/hello.py";
	const char* argv[] = {"/usr/bin/python3", cgi_program, nullptr};

	execve(argv[0], const_cast<char* const*>(argv), const_cast<char* const*>(_env_vars));
	
	perror("execve failed");
	exit(EXIT_FAILURE);
}

// Function to handle CGI requests
void CGI::handle_cgi_request(int client_socket, const std::string& path, CGI cgi) {
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
}