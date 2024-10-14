#pragma once

#include <vector>
#include <iostream>
#include "Client.hpp"

class Client; 

class Server{
	private:
		const char* port;
		std::string server_name;
	public:
		std::vector<Client> clients; //do i need it?
		Server();
		~Server();
		Server& operator=(const Server& copy);
		Server(const Server& copy);


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

		void addClient(std::vector<struct pollfd> &pfds, int clientSocket);
		void removeClient(std::vector<struct pollfd> pfds, int i, int clientSocket);
		int handleRequest(int clientSocket, std::string request);

		void fork_and_handle_cgi(int client_socket, const std::string& cgi_script_path);
};
