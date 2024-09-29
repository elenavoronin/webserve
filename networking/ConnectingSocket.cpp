/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ConnectingSocket.cpp                               :+:    :+:            */
/*                                                     +:+                    */
/*   By: dreijans <dreijans@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/09/29 16:51:54 by dreijans      #+#    #+#                 */
/*   Updated: 2024/09/29 17:07:58 by dreijans      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectingSocket.hpp"

//Constructor
ConnectingSocket::ConnectingSocket(int domain, int service, int protocol, int port, u_long interface) : SimpleSocket(domain, service, protocol, port, interface) {
	//establish network connection
	set_connection(connectToNetwork(get_sock(), get_address()));
	testConnection(get_connection());
}

//Destructor
ConnectingSocket::~ConnectingSocket() {
	// Clean up if necessary (parent class handles closing the socket)
    std::cout << "ConnectingSocket Destructor: Cleaning up resources." << std::endl;
}

//definition of connectToNetwork virtual funtion, Connect
int ConnectingSocket::connectToNetwork(int sock, struct sockaddr_in address) {
	int connectResult = connect(sock, (struct sockaddr*)&address, sizeof(address));
	testConnect(connectResult);
	return connectResult;
}

//Connect checker
int ConnectingSocket::testConnect(int connect) {
	if (connect < 0) {
        perror("Failed to connect the socket");
        exit(EXIT_FAILURE);
    }
}