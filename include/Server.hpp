
#pragma once
#include <vector>
#include <iostream>
// #include "Client.hpp"
#include <sstream>
#include <fstream>

class Client; 
class Location;

class Server{
	private:
		const char* 				_port;
		std::string 				_server_name;
		std::string                 _root;
        std::vector<std::string>    _allowed_methods;
        std::string                 _cgi_pass;
        std::string					_cgi_path;
		bool                        _autoindex;
        std::string                 _upload_store;
        std::string                 _default_file;
		std::string					_host;			   
		std::vector<Location> 		_locations;
		std::vector<std::string>	_errorPage;
	public:
		std::vector<Client> clients; //do i need it?
		Server();
		Server(const Server& copy);
		~Server();

		void run();
		/*listener socket*/
		int 	report_ready(std::vector<struct pollfd> &pfds);
		int		get_listener_socket();
		int 	sendall(int s, char *buf, int *len);
		/*Fd management*/
		void 	add_to_pfds(std::vector<struct pollfd> &pfds, int newfd);
		void 	del_from_pfds(std::vector<struct pollfd> &pfds, int i);
		/*Main loop*/
		void 	handle_new_connection(int listener, std::vector<struct pollfd> &pfds);
		void 	handle_client_data(std::vector<struct pollfd> &pfds, int i, int listener);
		void 	broadcast_message(int sender_fd, char *buf, int received, std::vector<struct pollfd> &pfds, int listener);

		void 	addClient(std::vector<struct pollfd> &pfds, int clientSocket);
		void 	removeClient(std::vector<struct pollfd> pfds, int i, int clientSocket);
		int handleRequest(int clientSocket, std::string request);

		//setters
     	void set_server_name(const std::string &server_name) { _server_name = server_name; }
        void set_port(const char* port) { _port = port; }
        void set_root(const std::string &root) { _root = root; }
        void set_autoindex(bool autoindex) { _autoindex = autoindex; }
        void set_cgi_pass(const std::string &cgi_pass) { _cgi_pass = cgi_pass; }
		void set_cgi_path(const std::string &cgi_path) { _cgi_path = cgi_path; }
        void set_upload_store(const std::string &upload_store) { _upload_store = upload_store; }
        void set_allowed_methods(const std::vector<std::string> &allowed_methods) { _allowed_methods = allowed_methods; }
        void set_default_file(const std::string &default_file) { _default_file = default_file; }

		//getter
		std::string getCgiPass() { return _cgi_pass; }
		std::string getCgiPath() { return _cgi_path; }

	    //for debugging only

        void print_info() const {
        std::cout << "Server Name: " << _server_name << std::endl;
        std::cout << "Port: " << (_port ? _port : "None") << std::endl;
        std::cout << "Root: " << _root << std::endl;
        std::cout << "Allowed Methods: ";
       for (std::vector<std::string>::const_iterator it = _allowed_methods.begin(); it != _allowed_methods.end(); ++it) {
    			std::cout << *it << " ";
        	}
        std::cout << std::endl;
		std::cout << "Cgi path: " << (_cgi_pass) << std::endl;
		}
    

};
