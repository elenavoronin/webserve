
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>


#define PORT "3490" //change to 80?
#define BACKLOG 10
#define MAXDATASIZE 100 //max number of bytes we cat get at once

int main(int argc, char *argv[]){
	int status;
	int sockfd;
	int readbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints;
	struct addrinfo *servinfo;
	struct addrinfo *newConnect;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0){
		std::cerr << "Client: getaddrinfo error: " << gai_strerror(status) << std::endl;
		return 1;
	}
	for (newConnect = servinfo; newConnect != NULL; newConnect = newConnect->ai_next){
		if ((sockfd = socket(newConnect->ai_family, newConnect->ai_socktype, newConnect->ai_protocol)) == -1){
		std::cerr << "Client: Socket create error " << std::endl;
		return 1;			
		}
		if (connect(sockfd, newConnect->ai_addr, newConnect->ai_addrlen) == -1){
			close(sockfd);
			std::cerr << "Client: connect error" << std::endl;
			continue;
		}
		break;
	}
	if (newConnect == NULL){
		std::cerr << "Client: failed to connect 2??? " << std::endl;
		exit(1);		
	}
	freeaddrinfo(servinfo);
	if ((readbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1){
		std::cerr << "Client: failed to connect 2??? " << std::endl;
		exit(1);
	}
	buf[readbytes] = '\0';
	std::cout << "Client received: " << buf << std::endl;
	close(sockfd);
	return 0;

}