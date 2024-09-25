/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   BindingSocket.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: dreijans <dreijans@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/09/25 15:51:47 by dreijans      #+#    #+#                 */
/*   Updated: 2024/09/25 16:00:18 by dreijans      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "BindingSocket.hpp"

BindingSocket::BindingSocket(int domain, int service, int protocol, int port, u_long interface) : SimpleSocket(domain, service, protocol, port, interface) {
	
}