#include "../Server.hpp"
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

void Server::handle_new_connection(int listener, std::vector<struct pollfd> &pfds){
    struct sockaddr_storage clientsAddr;
    socklen_t clientsAddrSize = sizeof(clientsAddr);
    int newfd = accept(listener, (struct sockaddr *)&clientsAddr, &clientsAddrSize);
    if (newfd == -1) {
        perror("accept");
    } else {
        add_to_pfds(pfds, newfd);
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
	char buf[256] = {0};
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
		del_from_pfds(pfds, i);
		}
	else
		broadcast_message(sender_fd, buf, received, pfds, listener);
}
