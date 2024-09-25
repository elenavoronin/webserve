/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   BindingSocket.hpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: dreijans <dreijans@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/09/25 15:51:50 by dreijans      #+#    #+#                 */
/*   Updated: 2024/09/25 16:42:26 by dreijans      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "SimpleSocket.hpp"
#include <iostream>

class BindingSocket : public SimpleSocket {

	private:

	public:
		
		BindingSocket(int domain, int service, int protocol, int port, u_long interface);
		BindingSocket(const SimpleSocket &copy) = delete;
		BindingSocket& operator=(const SimpleSocket &copy) = delete;
		~BindingSocket();
		
		int 		connectToNetwork(int sock, struct sockaddr_in address) override;
		int 		testBind(int);
	
};