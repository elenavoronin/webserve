/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   SimpleSocket.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: dreijans <dreijans@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/09/23 16:02:20 by dreijans      #+#    #+#                 */
/*   Updated: 2024/09/23 17:03:32 by dreijans      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "SimpleSocket.hpp"

/**
 * why pass address,
 * what is sin_port
 * what is sin_family
 * what does htons do?
 * what does the sock entail?
**/

//Default constructor
SimpleSocket::SimpleSocket(int domain, int service, int protocol, int port, u_long interface){
	//define address structure
	_address.sin_port = htons(port); //replace by setters?
	_address.sin_family = domain; //replace by setters?
	_address.sin_addr.s_addr = htonl(interface); //replace by setters?
	//establish socket
	_sock = socket(domain, service, protocol); //replace by setters?
	testConnection(_sock);
	//establish network connection
	_connection = connectToNetwork(_sock, _address);//call bind or connect
	testConnection(_connection);
}

//test connection virtual function
void SimpleSocket::testConnection(int item_to_test) {
	//confirm if connection is established
	if (item_to_test < 0) {
		perror("Failed to connect");
		exit(EXIT_FAILURE);
	}
}

//getter functions
struct sockaddr_in SimpleSocket::get_address() {
	return _address;
}

int SimpleSocket::get_sock() {
	return _sock;
}

int SimpleSocket::get_connection() {
	return _connection;
}

