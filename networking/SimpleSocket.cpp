/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   SimpleSocket.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: dreijans <dreijans@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/09/23 16:02:20 by dreijans      #+#    #+#                 */
/*   Updated: 2024/09/23 16:26:56 by dreijans      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "SimpleSocket.hpp"

// why pass address,
// what is sin_port
// what is sin_family
// what does htons do?
// what does the connection_fd entail?
SimpleSocket::SimpleSocket(int domain, int service, int protocol, int port, u_long interface){
	//define address structure
	address.sin_port = htons(port); 
	address.sin_family = domain;
	address.sin_addr.s_addr = htonl(interface);
	//establish connection
	connection_fd = socket(domain, service, protocol);
	//bind?
}