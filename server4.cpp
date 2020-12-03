#include "libraries.h"
#include "RoutingTable.h"
using namespace std;

int main() {
	RTRow rt[10];
	cPort cPortArr[10];
	int rtCounter=0;

	//open a socket for clients, bind that socket to portNo
	int reqListenFD = bindSock2Port(localhost, S4PORTNO);

	//establish tcp connection with server3
	int sockfd_forS3 = connectSock2Port(localhost, S3PORTNO);

	//Define fdset collection
	fd_set readfds;
	
	//Define fds array
	int readfds_arr[MAXFD];
	for(int i=0; i<MAXFD; ++i) {
		readfds_arr[i]  = -1;
	}
	
	//Add a file descriptor to the fds array
	fds_init(readfds_arr,  reqListenFD);
	fds_init(readfds_arr,  sockfd_forS3);

	while(1) {
		FD_ZERO(&readfds);//Clear the readfds array to 0
		int maxfd = -1;

		//For loop finds the maximum subscript for the ready event in the fds array
		for(int i=0; i<MAXFD; i++)	{
			if(readfds_arr[i] == -1) {
				continue;
			}
			FD_SET(readfds_arr[i],  &readfds);
			if(readfds_arr[i] > maxfd) {
				maxfd = readfds_arr[i];
			}
		}

		struct timeval tv = {15, 0};
		//select system call, where we only focus on read events
		int retval = select(maxfd+1, &readfds, NULL, NULL, &tv);
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
					if(readfds_arr[i] == reqListenFD) {
						//Accept new client connections
						struct sockaddr_in caddr;
						socklen_t len=sizeof(caddr);
						//Accept new client connections
						retval = accept(reqListenFD,(struct sockaddr *)&caddr, &len);
						
						if(retval < 0) {
							continue;
						} else if(retval > MAXFD+2) {
							printf("WARNING: Server limit exceeded\nrequest declined!\n");
							continue;
						}
						cPortArr[retval].portNo = (int)ntohs(caddr.sin_port);
						// cPortArr[retval].available=false;
						printf("accept c=%d\n",retval);
						fds_init(readfds_arr, retval);//Add the connection socket to the readfds_arr array
					} else {  //Receive data recv when an existing client sends data
						char buff[BUFFSIZE] = {0};
						int res = recv(readfds_arr[i], buff, BUFFSIZE, 0);
						if(res <= 0) {
							close(readfds_arr[i]);
							for(int it1=0; it1<rtCounter; it1++){
								if(rt[it1].cPortNo == cPortArr[readfds_arr[i]].portNo){
									string delMsg = "--	" + to_string(rt[it1].cPortNo);
									char todelete[delMsg.size()+1];
									strcpy(todelete, delMsg.c_str());

									send(sockfd_forS3,todelete,sizeof(todelete),0);
									rt[it1].cPortNo=0;
									cPortArr[readfds_arr[i]].portNo=0;
									cPortArr[readfds_arr[i]].available=true;
									dispRT(rt, rtCounter);
									break;
								}
							}
							readfds_arr[i]=-1;
							printf("client disconnected!\n");
						} else {
							string str = string(buff);
							//new client is sending his detail to dedicated server
							if(buff[0]=='.') {
								if(cPortArr[readfds_arr[i]].available){
									string str2 = "self";
									string substr = str.substr(1,str.size()-1);
									rt[rtCounter].cName.replace(0,substr.size()-1,substr);
									rt[rtCounter].cPortNo = cPortArr[readfds_arr[i]].portNo;
									cPortArr[readfds_arr[i]].available = false;
									rt[rtCounter].nextSName.replace(0, 5, str2);
									rt[rtCounter].nextFD = readfds_arr[i];
									// rt[rtCounter].display();
									//info coming from server about new client
									string tosend = "-	" + rt[rtCounter].cName + "	" 
															+ to_string(rt[rtCounter].cPortNo) + "	server4";
									// char msg[tosend.size()+1]="-	usama	12345	server1";
									char msg[tosend.size()+1];
									strcpy(msg, tosend.c_str());
									send(sockfd_forS3, msg, sizeof(msg), 0);	
									rtCounter++;								
									dispRT(rt,rtCounter);
								}
								else{
									printf("client msg: %s\n",buff);									
								}
								//when other server wants to tell  that it have new client
							} else if(buff[0]=='-' && buff[1] == '\t') {
								char *token = strtok(buff, "\t"); 
								string tokArr[4];
								int tokArrCount = 0;
								while (token != NULL) {
									tokArr[tokArrCount] = string(token);
									tokArrCount++;
									printf("%s\n", token); 
									token = strtok(NULL, "\t"); 
								}
								rt[rtCounter].cName.replace(0,tokArr[1].size()-1,tokArr[1]);
								rt[rtCounter].cPortNo=stoi(tokArr[2]);
								rt[rtCounter].nextSName.replace(0,tokArr[3].size()-1,tokArr[3]);
								rt[rtCounter].nextFD=readfds_arr[i];
								rtCounter++;								
								dispRT(rt,rtCounter);
								printf("client msg: %s\n",buff);
								//info coming from server that its client is disconnected
							} else if(buff[0]=='-' && buff[1] == '-') {
								char *token = strtok(buff, "\t"); 
								string tokArr[2];
								int tokArrCount=0;
								while (token != NULL) {
									tokArr[tokArrCount] = string(token);
									tokArrCount++;
									printf("%s\n", token); 
									token = strtok(NULL, "\t"); 
								}
								int toDeletePortNo = stoi(tokArr[1]);
								for(int it1 = 0; it1 < rtCounter; it1++) {
									if(rt[it1].cPortNo == toDeletePortNo) {
										rt[it1].cPortNo = 0;
										break;
									}
								}
								dispRT(rt,rtCounter);
							} else {//client 2 client communication
								char buffTemp[BUFFSIZE];
								strcpy(buffTemp,buff);
								char *token = strtok(buffTemp, "\t"); 
								string tokArr[3];
								int tokArrCount=0;
								while (token != NULL) {
									tokArr[tokArrCount] = string(token);
									tokArrCount++;
									token = strtok(NULL, "\t"); 
								}
								bool isConnected = false;
								for(int it1 = 0; it1 < rtCounter; it1++) {
									if(rt[it1].cPortNo == 0) {
										continue;
									}
									if(rt[it1].cName == tokArr[1]) {
										send(rt[it1].nextFD, buff, sizeof(buff),0);
										isConnected = true;
									}
								}
								if (!isConnected) {
									for(int it1 = 0; it1 < rtCounter; it1++) {
										if(rt[it1].cPortNo == 0) {
											continue;
										}
										if(rt[it1].cName == tokArr[0]) {
											string str = string(rt[it1].cName) + "\t"
												+ string(tokArr[1]) + "\t" + "client not found";
											send(rt[it1].nextFD, str.c_str(), str.length(), 0);
										}
									}
								}
								
							}
							
						}
					}
				}
			}
		
		}
	}
}
