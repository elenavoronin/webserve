/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   SimpleSocket.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: dreijans <dreijans@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/09/23 16:02:20 by dreijans      #+#    #+#                 */
/*   Updated: 2024/09/24 16:49:47 by dreijans      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "SimpleSocket.hpp"

//Default constructor
SimpleSocket::SimpleSocket(int domain, int service, int protocol, int port, u_long interface){
	//define address structure
	set_port(port);
	set_family(domain);
	set_address(interface);
	//establish socket
	_sock = socket(domain, service, protocol);
	testConnection(_sock);
	//establish network connection
	_connection = connectToNetwork(_sock, _address);//call bind or connect
	testConnection(_connection);
}

//deconstructor
SimpleSocket::~SimpleSocket() {
	if (_sock > 0) {
		close(_sock);
		std::cout << "Socket closed in destructor." << std::endl;
	}
}

//setter functions
void SimpleSocket::set_port(int port) {
	_address.sin_port = htons(port);
}

void SimpleSocket::set_family(int domain) {
	_address.sin_family = domain;
}

void SimpleSocket::set_address(u_long interface) {
	_address.sin_addr.s_addr = htonl(interface);
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
