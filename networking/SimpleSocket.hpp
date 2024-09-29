/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   SimpleSocket.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: dreijans <dreijans@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/09/23 16:00:32 by dreijans      #+#    #+#                 */
/*   Updated: 2024/09/29 16:48:00 by dreijans      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "sys/socket.h"
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>  // for close()

// int server_fd = socket(domain, type, protocol);
// domain = type of IP address
// type = type of service
// protocol = to use and support the socket operation, dependend on type and domain

class SimpleSocket {
	private:

		struct sockaddr_in 	_address;
		int 				_sock;
		int					_connection;
		
	public:
		
		SimpleSocket(int domain, int service, int protocol, int port, u_long interface);
		SimpleSocket(const SimpleSocket &copy) = delete;
		SimpleSocket& operator=(const SimpleSocket &copy) = delete;
		~SimpleSocket();

		//virtual fucntion to connect to a network
		virtual int 		connectToNetwork(int sock, struct sockaddr_in address) = 0;
		
		//function to test sockets and connections
		void 				testConnection(int); //maybe virtual function?
		
		//setter functions
		void				set_port(int port);
		void				set_family(int domain);
		void				set_address(u_long interface);
		void				set_connection(int connection);
		
		//getter functions
		struct sockaddr_in 	get_address();
		int 				get_sock();
		int 				get_connection();
};