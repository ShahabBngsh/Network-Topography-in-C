#include "libraries.h"
using namespace std;

#define MAXFD 4	//Size of fds array
#define PORTNO 6000
#define backlog 3
#define localhost "127.0.0.1"
#define BUFFSIZE 127
struct RoutingTableRow{
	// string client_name;
	string client_portNo;
	string next_serverName;
	float next_FD;
};


int main() {
	RoutingTableRow routingTable[10];
	int routingTable_Counter=0;
	int connectedServerFD=0;
	string newClient="client";
	string isServer="6000";
	int sockfd=socket(AF_INET,SOCK_STREAM,0);
	checkError(sockfd, "Error socket");
  
	struct sockaddr_in saddr,caddr;
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(PORTNO);
	saddr.sin_addr.s_addr=inet_addr(localhost);

	int res=bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
	checkError(res, "Error bind");
	
	//Create listening queue
	listen(sockfd, backlog);
	//Define fdset collection
	fd_set readfds_set, writefds_set;
	
	//Define fds array
	int readfds_arr[MAXFD], writefds_arr[MAXFD];
	for(int i=0; i<MAXFD; ++i) {
		readfds_arr[i]  = -1;
		writefds_arr[i] = -1;
	}
	
	//Add a file descriptor to the fds array
  fds_init(readfds_arr,  sockfd);
	fds_init(writefds_arr, sockfd);

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
					if(readfds_arr[i]==sockfd) {

						//Accept new client connections
							struct sockaddr_in caddr;
							socklen_t len=sizeof(caddr);
	//Accept new client connections
						int retval =accept(sockfd,(struct sockaddr *)&caddr, &len);
						// int retval = acceptClient(sockfd);
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
						int res=recv/*from*/(readfds_arr[i], buff, sizeof(buff),0/*, (struct sockaddr*)&cliaddr[0],&len*/);
						//int res=recv(readfds_arr[i],buff,BUFFSIZE,0);
						if(res <= 0) {
							close(readfds_arr[i]);
							readfds_arr[i]=-1;
							printf("client disconnected!\n");
						} else {
							printf("recv(%d)=%s\n", readfds_arr[i], buff);	//Output Client Sent Information
							string str(buff);
							bool client=true;
							if(buff[0]=='.'){
								routingTable[routingTable_Counter].client_portNo=str.substr(1,str.size());
								routingTable[routingTable_Counter].next_serverName="self";
								routingTable[routingTable_Counter].next_FD=readfds_arr[i];
								routingTable_Counter++;
								string tosend_newclient="-."+str.substr(1,str.size()); 	
								//send(connectedServerFD,tosend_newclient, 0)													
							}
							else if(buff[0]=='-'&&buff[1]=='.'){
								routingTable[routingTable_Counter].client_portNo=str.substr(2,str.size());
								routingTable[routingTable_Counter].next_serverName="server3";
								routingTable[routingTable_Counter].next_FD=readfds_arr[i];
								routingTable_Counter++;
							}
							else if(buff[0]=='-'){
								connectedServerFD=readfds_arr[i];
							}

							// for(int it1=0;it1<5;it1++){
							// 	if(str[it1]!=isClient[it1]){
							// 		client=false;
							// 	}
							// }
							// if(client){
							// 	// routingTable[routingTable_Counter].client_name=buff;
							// 	routingTable[routingTable_Counter].next_serverName="self";
							// 	routingTable[routingTable_Counter].next_FD=readfds_arr[i];
							// 	routingTable_Counter++;								
							// }
							if(str!=isServer){
								
							}
							// //send(readfds_arr[i],"",0,0);
							// char arr[ ] = "This is a test";
							// string str(arr);
							// //  You can also assign directly to a string.
							// str = "This is another string";
							// str = arr;
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
					if(writefds_arr[i]==sockfd) {
						//Accept new client connections
						int retval = acceptClient(sockfd);
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
// sendto(sockfd, message, MAXLINE, 0, (struct sockaddr*)&servaddr, sizeof(servaddr)); 
// int n = recvfrom(listenfd, buffer, sizeof(buffer), 
// 			0, (struct sockaddr*)&cliaddr[0],&len); //receive message from server