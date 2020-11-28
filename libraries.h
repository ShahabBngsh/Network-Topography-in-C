//all required libraries
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<string.h>

#include <string>
#include <iostream>
#include <functional>

#define localhost "127.0.0.1"
#define MSPORTNO 7000
#define backlog 3
#define MAXFD 4	//Size of fds array
#define BUFFSIZE 127


//Add a file descriptor to the fds array
void fds_init(int fds_arr[],int fd) {
	for(int i=0; i<MAXFD; ++i) {
		if(fds_arr[i]==-1) {
			fds_arr[i]=fd;
			break;
		}
	}
}

void checkError(const int& ret, const char* s) {
	if(ret == -1) {
		perror(s);
		exit(-1);
	}
}
int acceptClient(int sockfd) {
	struct sockaddr_in caddr;
	socklen_t len=sizeof(caddr);
	//Accept new client connections
	return accept(sockfd,(struct sockaddr *)&caddr, &len);
}