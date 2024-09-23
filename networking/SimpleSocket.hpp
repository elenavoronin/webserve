/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   SimpleSocket.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: dreijans <dreijans@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/09/23 16:00:32 by dreijans      #+#    #+#                 */
/*   Updated: 2024/09/23 16:27:14 by dreijans      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "sys/socket.h"
#include <netinet/in.h>

// int server_fd = socket(domain, type, protocol);
// domain = type of IP address
// type = type of service
// protocol = to use and support the socket operation, dependend on type and domain

class SimpleSocket {
	private:

		struct sockaddr_in 	address;
		int 				connection_fd;
		
	public:
	
		SimpleSocket(int domain, int service, int protocol, int port, u_long interface);
		SimpleSocket(const SimpleSocket &copy);
		SimpleSocket& operator=(const SimpleSocket &copy);
		~SimpleSocket();

		void setSimpleSocket();
		?? getSimpleSocket();
};