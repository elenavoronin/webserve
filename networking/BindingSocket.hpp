/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   BindingSocket.hpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: dreijans <dreijans@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/09/25 15:51:50 by dreijans      #+#    #+#                 */
/*   Updated: 2024/09/29 19:32:06 by dreijans      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

/**
 * This class implements a call to bind() function of 
 * sockets as a definition of the connectToNetwork
 * virtual function form SimpleSocket
 */

#pragma once

#include "SimpleSocket.hpp"
#include <iostream>

class BindingSocket : public SimpleSocket {

	private:

	public:
		//constructor
		BindingSocket(int domain, int service, int protocol, int port, u_long interface);
		BindingSocket(const SimpleSocket &copy) = delete;
		BindingSocket& operator=(const SimpleSocket &copy) = delete;
		//destructor
		~BindingSocket();
		
		//virtual function from parent
		int	connectToNetwork(int sock, struct sockaddr_in address) override;
		int	testBind(int);
	
};