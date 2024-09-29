/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ListeningSocket.hpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: dreijans <dreijans@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/09/29 17:51:53 by dreijans      #+#    #+#                 */
/*   Updated: 2024/09/29 19:32:01 by dreijans      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include "BindingSocket.hpp"

class ListeningSocket : public BindingSocket {

	private:

		int _backlog;

	public:

		ListeningSocket(int domain, int service, int protocol, int port, u_long interface, int bcklog);
		ListeningSocket(const ListeningSocket &copy) = delete;
		ListeningSocket& operator=(const ListeningSocket &copy) = delete;
		~ListeningSocket();
		
			
};