/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   BindingSocket.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: dreijans <dreijans@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/09/25 15:51:47 by dreijans      #+#    #+#                 */
/*   Updated: 2024/09/25 16:48:47 by dreijans      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "BindingSocket.hpp"

//Constructor
BindingSocket::BindingSocket(int domain, int service, int protocol, int port, u_long interface) : SimpleSocket(domain, service, protocol, port, interface) {
	std::cout << "BindingSocket: Socket successfully bound." << std::endl;
}

//Destructor
BindingSocket::~BindingSocket() {
	// Clean up if necessary (parent class handles closing the socket)
    std::cout << "BindingSocket Destructor: Cleaning up resources." << std::endl;
}

//binding method
int BindingSocket::connectToNetwork(int sock, struct sockaddr_in address) {
	int bindResult = bind(sock, (struct sockaddr*)&address, sizeof(address));
	testBind(bindResult);
	return bindResult;
}

//binding checker
int BindingSocket::testBind(int bind) {
	if (bind < 0) {
        perror("Failed to bind the socket");
        exit(EXIT_FAILURE);
    }
}
