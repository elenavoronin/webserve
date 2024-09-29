/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ConnectingSocket.hpp                               :+:    :+:            */
/*                                                     +:+                    */
/*   By: dreijans <dreijans@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/09/29 16:52:14 by dreijans      #+#    #+#                 */
/*   Updated: 2024/09/29 17:05:25 by dreijans      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

/**
 * This class implements a call to connect() function of 
 * sockets as a definition of the connectToNetwork
 * virtual function form SimpleSocket
 */

#pragma once

#include "SimpleSocket.hpp"
#include <iostream>

class ConnectingSocket : public SimpleSocket {
	private:

	public:
		ConnectingSocket(int domain, int service, int protocol, int port, u_long interface);
		ConnectingSocket(const SimpleSocket &copy) = delete;
		ConnectingSocket& operator=(const SimpleSocket &copy);
		~ConnectingSocket();
		
		int	connectToNetwork(int sock, struct sockaddr_in address) override;
		int	testConnect(int);		
};