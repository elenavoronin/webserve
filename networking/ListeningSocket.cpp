/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ListeningSocket.cpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: dreijans <dreijans@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/09/29 17:51:50 by dreijans      #+#    #+#                 */
/*   Updated: 2024/09/29 19:34:30 by dreijans      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "ListeningSocket.hpp"

ListeningSocket::ListeningSocket(int domain, int service, int protocol, int port, u_long interface, int bcklog) : BindingSocket(domain, service, protocol, port, interface) {
	_backlog = bcklog;
}

