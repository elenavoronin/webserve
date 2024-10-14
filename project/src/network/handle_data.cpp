#include "../Server.hpp"
#include "../Client.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <iostream>
#include <vector>

void Server::addClient(std::vector<struct pollfd> &pfds, int clientSocket){
	Client* newClient = new Client();
	newClient->setSocket(clientSocket);
	//	clients.push_back(newClient); //do I need it?
	add_to_pfds(pfds, clientSocket);
	std::cout << "Add client. ClientSocket: " << clientSocket << std::endl;
}

void Server::removeClient(std::vector<struct pollfd> pfds, int i, int clientSocket){
	del_from_pfds(pfds, i);
	// for (auto it = clients.begin(); it != clients.end(); ++it) {
	// 	if (it->getSocket() == clientSocket) {
	// 		clients.erase(it);
	// 		break;
	// 	}
	// }
	// close(clientSocket);
}

void Server::handle_new_connection(int listener, std::vector<struct pollfd> &pfds){
	struct sockaddr_storage clientsAddr;
    socklen_t clientsAddrSize = sizeof(clientsAddr);
    int newfd = accept(listener, (struct sockaddr *)&clientsAddr, &clientsAddrSize);
    if (newfd == -1) {
        perror("accept");
    } else {
		addClient(pfds, newfd);
		
        //add_to_pfds(pfds, newfd);
        std::cout << "New connection. Newfd: " << newfd << std::endl;
    }
}

void Server::broadcast_message(int sender_fd, char *buf, int received, std::vector<struct pollfd> &pfds, int listener){
    for (size_t j = 0; j < pfds.size(); j++) {
        int dest_fd = pfds[j].fd;
        if (dest_fd != listener && dest_fd != sender_fd) {
            if (sendall(dest_fd, buf, &received) == -1) {
                std::cout << "Can't send" << std::endl;
            }
        }
    }
}

int Server::sendall(int s, char *buf, int *len){
	int total = 0;
	int bytesleft = *len;
	int n;

	while(total < *len){
		n = send(s, buf + total, bytesleft, 0);
		if (n == -1)
			break;
		total += n;
		bytesleft -= n;
	}
	*len = total;
	return (n == -1) ? -1 : 0; 
}

void Server::handle_client_data(std::vector<struct pollfd> &pfds, int i, int listener){
	char buf[1024] = {0}; //problemo for big requests
	int received = recv(pfds[i].fd, buf, sizeof buf, 0);
	std::cout << "Received bytes: " << received << std:: endl;
	int sender_fd = pfds[i].fd;
	if (received <= 0) {
		if (received == 0) {
			std::cout << "pollserver: socket" << sender_fd << "hung up\n";
		} else {
			perror("recv");
		}
		close(sender_fd);
		// del_from_pfds(pfds, i);
		removeClient(pfds, i, sender_fd);
		}
	else {
		buf[received] = '\0';
		std::string request(buf);
		if (handleRequest(sender_fd, request) != 0)
			broadcast_message(sender_fd, buf, received, pfds, listener);
		del_from_pfds(pfds, i); //should it be here?
	}
}
