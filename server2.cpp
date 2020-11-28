#include "libraries.h"
using namespace std;

#define MAXFD 4	//Size of fds array
#define MSPORTNO 6000
#define PORTNO 7000
#define backlog 3
#define localhost "127.0.0.1"
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

int main() {

  // for connection with main server
  int sockfd_ms = socket(AF_INET, SOCK_STREAM, 0);
	checkError(sockfd_ms, "Error socket");
  struct sockaddr_in msaddr;
	memset(&msaddr, 0, sizeof(msaddr));
	msaddr.sin_family = AF_INET;
	msaddr.sin_port = htons(MSPORTNO);
	msaddr.sin_addr.s_addr = inet_addr(localhost);

  int retval = connect(sockfd_ms, (struct sockaddr*)&msaddr, sizeof(msaddr));
	checkError(retval, "Error connect");

  // for connection with clients
	int sockfd_self = socket(AF_INET, SOCK_STREAM, 0);
	checkError(sockfd_self, "Error socket");
  
	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(PORTNO);
	saddr.sin_addr.s_addr = inet_addr(localhost);

	int res = bind(sockfd_self,(struct sockaddr*)&saddr,sizeof(saddr));
	checkError(res, "Error bind");
	
	//Create listening queue
	listen(sockfd_self, backlog);
	//Define fdset collection
	fd_set readfds, writefds;
	
	//Define fds array
	int readfds_arr[MAXFD], writefds_arr[MAXFD];
	for(int i=0; i<MAXFD; ++i) {
		readfds_arr[i]  = -1;
		writefds_arr[i] = -1;
	}
	
	//Add a file descriptor to the fds array
  fds_init(readfds_arr,  sockfd_self);
	fds_init(writefds_arr, sockfd_self);

	while(1) {
		FD_ZERO(&readfds);//Clear the readfds array to 0
		FD_ZERO(&writefds);//Clear the writefds array to 0
		int maxfd=-1;

		//For loop finds the maximum subscript for the ready event in the fds array
		for(int i=0; i<MAXFD; i++)	{
			if(readfds_arr[i]==-1) {
				continue;
			}
			FD_SET(readfds_arr[i],  &readfds);
			if(readfds_arr[i] > maxfd) {
				maxfd = readfds_arr[i];
			}
		}
		for(int i=0; i<MAXFD; i++)	{
			if(writefds_arr[i]==-1) {
				continue;
			}
			FD_SET(writefds_arr[i],  &writefds);
			if(writefds_arr[i] > maxfd) {
				maxfd = writefds_arr[i];
			}
		}

		struct timeval tv = {15, 0};
		//Selectect system call, where we only focus on read events
		int retval = select(maxfd+1, &readfds, &writefds, NULL,&tv);
		if(retval == -1)	{ //fail
			perror("Error select()");
		}
		else if(retval == 0) { //Timeout, meaning no file descriptor returned
			printf("time out\n");
		}
		else { //Ready event generation
			//loop through each readfd, to see if it has something to read from
			for(int i=0; i < MAXFD; ++i)	{
				if(readfds_arr[i]==-1) {	//If readfds_arr[i]==-1, the event is not ready
					continue;
				}//Determine if the event corresponding to the file descriptor is ready
				if(FD_ISSET(readfds_arr[i], &readfds)) {
					//accept if a new client requests a connection
					if(readfds_arr[i]==sockfd_self) {

						//Accept new client connections
						int retval = acceptClient(sockfd_self);
						if(retval < 0) {
							continue;
						} else if(retval > MAXFD+2) {
							printf("WARNING: Server limit exceeded\nrequest declined!\n");
							continue;
						}
						printf("accept c=%d\n",retval);
						fds_init(readfds_arr, retval);//Add the connection socket to the readfds_arr array
					} else {  //Receive data recv when an existing client sends data
						char buff[BUFFSIZE]={0};
						int res=recv(readfds_arr[i],buff,BUFFSIZE,0);
						if(res <= 0) {
							close(readfds_arr[i]);
							readfds_arr[i]=-1;
							printf("client disconnected!\n");
						} else {
							printf("recv(%d)=%s\n", readfds_arr[i], buff);	//Output Client Sent Information
							//send(readfds_arr[i],"",0,0);
							
						}
					}
				}
			}
			//loop through each writefd, to see if it has something to write to
			for(int i=0; i < MAXFD; ++i)	{
				if(writefds_arr[i]==-1) {	//If writefds_arr[i]==-1, the event is not ready
					continue;
				}//Determine if the event corresponding to the file descriptor is ready
				if(FD_ISSET(writefds_arr[i], &writefds)) {
					//accept if a new client requests a connection
					if(writefds_arr[i]==sockfd_self) {
						//Accept new client connections
						int retval = acceptClient(sockfd_self);
						if(retval < 0) {
							continue;
						}
						printf("accept c=%d\n",retval);
						fds_init(writefds_arr, retval);//Add the connection socket to the readfds_arr array
					} else {  //Receive data recv when an existing client sends data
						char buff[BUFFSIZE] = "---lol---\n";
						int retval = send(writefds_arr[i], buff, sizeof(buff), 0);
						if(retval <= 0) {
							close(writefds_arr[i]);
							writefds_arr[i]=-1;
							printf("client disconnected!\n");
						} else {
							printf("sent bytes: %d  -> client: %d\n", retval, i);
						}
					}
				}
			}
		
		}
	}
}
