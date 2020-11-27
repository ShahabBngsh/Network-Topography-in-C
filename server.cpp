#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<assert.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<string>

#include <iostream>
using namespace std;

#define MAXFD 5	//Size of fds array
#define PORTNO 6000

int id = 0;
int tickarr[12]={0};

void fds_add(int fds[],int fd)	//Add a file descriptor to the fds array
{
	int i=0;
	for(;i<MAXFD;++i) {
		if(fds[i]==-1) {
	    	fds[i]=fd;
			break;
		}
	}
}

int main() {

	int sockfd=socket(AF_INET,SOCK_STREAM,0);
	string sndmsg = "Your ticket ID is: ";
	if (sockfd == -1) {
		cout << "ERROR: socket\n";
	}
	
  printf("sockfd=%d\n",sockfd);
    
	struct sockaddr_in saddr,caddr;
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(PORTNO);
	saddr.sin_addr.s_addr=inet_addr("127.0.0.1");

	int res=bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
	if (res == -1) {
		cout << "ERROR: bind\n";
	}
	
	
	//Create listening queue
	listen(sockfd,12);
	
   //Define fdset collection
    fd_set fdset;
	
	//Define fds array
    int fds[MAXFD];
    int i=0;
    for(;i<MAXFD;++i) {
		fds[i]=-1;
    }
	
	//Add a file descriptor to the fds array
    fds_add(fds,sockfd);

	while(1) {
		FD_ZERO(&fdset);//Clear the fdset array to 0

		int maxfd=-1;

		int i=0;

		//For loop finds the maximum subscript for the ready event in the fds array
		for(;i<MAXFD;i++)
		{
			if(fds[i]==-1)
			{
				continue;
			}

			FD_SET(fds[i],&fdset);

			if(fds[i]>maxfd)
			{
				maxfd=fds[i];
			}
		}

		struct timeval tv={5,0};	//Set timeout of 5 seconds

		int n=select(maxfd+1,&fdset,NULL,NULL,&tv);//Selectect system call, where we only focus on read events
		if(n==-1)	{ //fail
		
			perror("select error");
		}
		else if(n==0)//Timeout, meaning no file descriptor returned
		{
			printf("time out\n");
		}
		else//Ready event generation
		{
		//Because we only know the number of ready events by the return value of select, we don't know which events are ready.
		//Therefore, each file descriptor needs to be traversed for judgment
			for(i=0;i<MAXFD;++i)
			{
				if(fds[i]==-1)	//If fds[i]==-1, the event is not ready
				{
					continue;
				}
				if(FD_ISSET(fds[i],&fdset))	//Determine if the event corresponding to the file descriptor is ready
				{
				//There are two kinds of cases for judging file descriptors
					if(fds[i]==sockfd)	//A file descriptor is a socket, meaning accept if a new client requests a connection
					{
						//accept
						struct sockaddr_in caddr;
						socklen_t len=sizeof(caddr);
						//Accept new client connections
						int c=accept(sockfd,(struct sockaddr *)&caddr, &len);
						if(c<0) {
							continue;
						}
					
						printf("accept c=%d\n",c);
						fds_add(fds,c);//Add the connection socket to the array where the file descriptor is stored
					}
					else {  //Receive data recv when an existing client sends data
						char buff[128]={0};
						int res=recv(fds[i],buff,127,0);
						if(res<=0)
						{
							close(fds[i]);
							fds[i]=-1;
							printf("one client over\n");
						}
						else
						{
							printf("recv(%d)=%s\n",fds[i],buff);	//Output Client Sent Information
							if(!strcmp(buff, "reserve\n") && id<12) {
								sndmsg = "Your ticket ID is: ";
								sndmsg +=  to_string(id);
							
								tickarr[id] = fds[i];
								id++;								
								send(fds[i],sndmsg.c_str(),strlen(sndmsg.c_str()),0);
							}
							else if(!strcmp(buff, "exit")) {
								send(fds[i],"",0,0);
							}
							else if(id > 11) {
								sndmsg = "";
								for (int i = 0; i < 12; i++) {
									sndmsg += to_string(tickarr[i]);
								}
								sndmsg += '\n';
								sndmsg += "All tickets are solved\n";
								send(fds[i],sndmsg.c_str(), strlen(sndmsg.c_str()), 0);
								
							}
							else {
								send(fds[i],"Invalid input",13,0);
							}
							printf("id: %d\n", id);
							printf("Updated ticket array:\n");
							for (int i = 0; i < 12; i++)
							{
								printf("%d ", tickarr[i]);
							}
							printf("\n");
								
						}
					}
				}
			}
		}
	}
}