#include "libraries.h"
using namespace std;

#define MAXFD 3	//Size of fds array
#define PORTNO 6000
#define backlog 3
#define localhost "127.0.0.1"
#define BUFFSIZE 127

int id = 0;
int tickarr[backlog]={0};

void fds_init(int fds[],int fd) {	//Add a file descriptor to the fds array
	int i=0;
	for( ; i<MAXFD; ++i) {
		if(fds[i]==-1) {
			fds[i]=fd;
			break;
		}
	}
}

void checkError(const int& ret, const int& flag,  const string& text) {
	if(ret == -1) {
		cout << "ERROR: " << text << " flag: " << flag;
		exit(-1);
	}
}

int main() {

	int sockfd=socket(AF_INET,SOCK_STREAM,0);
	checkError(sockfd, errno, "socket\n");
  
	struct sockaddr_in saddr,caddr;
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(PORTNO);
	saddr.sin_addr.s_addr=inet_addr(localhost);

	int res=bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
	checkError(res, errno, "bind\n");
	
	//Create listening queue
	listen(sockfd, backlog);
	//Define fdset collection
	fd_set fdset;
	
	//Define fds array
	int fds[MAXFD];
	int i=0;
	for(;i<MAXFD;++i) {
		fds[i]=-1;
	}
	
	//Add a file descriptor to the fds array
  fds_init(fds,sockfd);

	while(1) {
		FD_ZERO(&fdset);//Clear the fdset array to 0
		int maxfd=-1;

		int i=0;
		//For loop finds the maximum subscript for the ready event in the fds array
		for( ;i<MAXFD;i++)	{
			if(fds[i]==-1) {
				continue;
			}

			FD_SET(fds[i],&fdset);

			if(fds[i]>maxfd) {
				maxfd=fds[i];
			}
		}

		struct timeval tv={5,0};	//Set timeout of 5 seconds

		int retval=select(maxfd+1,&fdset,NULL,NULL,&tv);//Selectect system call, where we only focus on read events
		if(retval==-1)	{ //fail
			perror("ERROR: select()");
		}
		else if(retval==0) { //Timeout, meaning no file descriptor returned
			printf("time out\n");
		}
		else { //Ready event generation
		//Because we only know the number of ready events by the return value of select, we don't know which events are ready.
		//Therefore, each file descriptor needs to be traversed for judgment
			for(i=0;i<MAXFD;++i)	{
				if(fds[i]==-1) {	//If fds[i]==-1, the event is not ready
					continue;
				}
				if(FD_ISSET(fds[i],&fdset)) {	//Determine if the event corresponding to the file descriptor is ready
				//There are two kinds of cases for judging file descriptors
					if(fds[i]==sockfd) {	//A file descriptor is a socket, meaning accept if a new client requests a connection
						struct sockaddr_in caddr;
						socklen_t len=sizeof(caddr);
						//Accept new client connections
						int retval = accept(sockfd,(struct sockaddr *)&caddr, &len);
						if(retval < 0) {
							continue;
						}
						printf("accept c=%d\n",retval);
						fds_init(fds, retval);//Add the connection socket to the array where the file descriptor is stored
					} else {  //Receive data recv when an existing client sends data
						char buff[128]={0};
						int res=recv(fds[i],buff,BUFFSIZE,0);
						checkError(res,errno, "recv\n");
						if(res == 0) {
							close(fds[i]);
							fds[i]=-1;
							printf("client disconnected!\n");
						}
						else
						{
							printf("recv(%d)=%s\n",fds[i],buff);	//Output Client Sent Information
							//send(fds[i],"",0,0);
							
							printf("id: %d\n", id);
							printf("Updated ticket array:\n");
							for (int i = 0; i < backlog; i++) {
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