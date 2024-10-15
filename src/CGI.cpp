#include "CGI.hpp"

//constructor
CGI::CGI(){
	std::cout<< "CGI constructor called" << std::endl;
}

CGI::~CGI(){
	std::cout<< "CGI destructor called" << std::endl;
}

void::CGI::read_input(){
	
}

//executable checkt argv[1] but have to pass the executable as first argument
void::CGI::execute_cgi() {

	const char* cgi_program = "./www/html/cgi-bin/hello.py";
	const char* argv[] = {"/usr/bin/python3", cgi_program, nullptr};

	execve(argv[0], const_cast<char* const*>(argv), const_cast<char* const*>(_env_vars));
	
	perror("execve failed");
	exit(EXIT_FAILURE);
}