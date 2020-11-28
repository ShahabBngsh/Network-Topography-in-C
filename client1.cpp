#include "libraries.h"
using namespace std;

#define S1PORT 6000
#define PORT 6100

int main() {

  // for connection with server
  int sockfd_s1 = socket(AF_INET, SOCK_STREAM, 0);
	checkError(sockfd_s1, "Error socket");
  struct sockaddr_in s1addr;
	memset(&s1addr, 0, sizeof(s1addr));
	s1addr.sin_family = AF_INET;
	s1addr.sin_port = htons(S1PORT);
	s1addr.sin_addr.s_addr = inet_addr(localhost);

  int retval = connect(sockfd_s1, (struct sockaddr*)&s1addr, sizeof(s1addr));
	checkError(retval, "Error connect");

  // for connection with clients
	int sockfd_self = socket(AF_INET, SOCK_STREAM, 0);
	checkError(sockfd_self, "Error socket");
  
	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(PORT);
	saddr.sin_addr.s_addr = inet_addr(localhost);

	int res = bind(sockfd_self,(struct sockaddr*)&saddr,sizeof(saddr));
	checkError(res, "Error bind");
	
	//Create listening queue
	listen(sockfd_self, backlog);
	//Define fdset collection
	fd_set readfds_set, writefds_set;
	
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
		FD_ZERO(&readfds_set);//Clear the readfds_set array to 0
		FD_ZERO(&writefds_set);//Clear the writefds_set array to 0
		int maxfd=-1;

		//For loop finds the maximum subscript for the ready event in the fds array
		for(int i=0; i<MAXFD; i++)	{
			if(readfds_arr[i]==-1) {
				continue;
			}
			FD_SET(readfds_arr[i],  &readfds_set);
			if(readfds_arr[i] > maxfd) {
				maxfd = readfds_arr[i];
			}
		}
		for(int i=0; i<MAXFD; i++)	{
			if(writefds_arr[i]==-1) {
				continue;
			}
			FD_SET(writefds_arr[i],  &writefds_set);
			if(writefds_arr[i] > maxfd) {
				maxfd = writefds_arr[i];
			}
		}

		struct timeval tv = {15, 0};
		//Selectect system call, where we only focus on read events
		int retval = select(maxfd+1, &readfds_set, &writefds_set, NULL,&tv);
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
				if(FD_ISSET(readfds_arr[i], &readfds_set)) {
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
				if(FD_ISSET(writefds_arr[i], &writefds_set)) {
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
