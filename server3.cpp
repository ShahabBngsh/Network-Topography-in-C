#include "libraries.h"
#include<cstdlib>
#include<stdlib.h>
using namespace std;

#define PORTNO 10000
struct RoutingTableRow{
	string clientName="";
	int client_PortNo=0;
	int next_toGoFD=-1;
	string next_serverName="";

	void display(){
		// printf("Clinet_PortNo: %d  NextServerName:%s  Next_toGoFd:%d\n",client_PortNo,next_serverName,next_toGoFD);
		cout<<"ClientName: "<<clientName<<"  Clinet_PortNo: "<<client_PortNo<<"  NextServerName :"<<next_serverName<<"  Next_toGoFd: "<<next_toGoFD<<endl;
	}
};

void displayRoutingTable(RoutingTableRow* routingTable,int routingtablecount){
	// system("clear");
	cout<<"***********************************************************\n";
	int counter=0;
	for(int it1=0;it1<routingtablecount;it1++){
		if(routingTable[it1].client_PortNo!=0){
			counter++;
			// cout<<endl<<it1<<endl;
			routingTable[it1].display();
		}
	}
	// cout<<counter<<endl;
	cout<<"***********************************************************\n";
}

int main() {
	RoutingTableRow routingTable[10];
	int routingTableCount=0;
  // for connection with clients
	int requestListenFD = socket(AF_INET, SOCK_STREAM, 0);
	checkError(requestListenFD, "Error socket");
    int server1FD=-1,server2FD=-1,server4FD=-1;
	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(PORTNO);
	saddr.sin_addr.s_addr = inet_addr(localhost);

	int res = bind(requestListenFD,(struct sockaddr*)&saddr,sizeof(saddr));
	checkError(res, "Error bind");
	
	//Create listening queue
	listen(requestListenFD, backlog);
	//Define fdset collection
	fd_set readfds_set;
	
	//Define fds array
	int readfds_arr[MAXFD];
	for(int i=0; i<MAXFD; ++i) {
		readfds_arr[i]  = -1;
	}
	
	//Add a file descriptor to the fds array
  fds_init(readfds_arr,  requestListenFD);

	while(1) {
		FD_ZERO(&readfds_set);//Clear the readfds_set array to 0
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
		struct timeval tv = {15, 0};
		//Selectect system call, where we only focus on read events
		int retval = select(maxfd+1, &readfds_set, NULL, NULL,&tv);
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
					if(readfds_arr[i]==requestListenFD) {
						//Accept new client connections
                        struct sockaddr_in caddr;
						socklen_t len=sizeof(caddr);
						//Accept new client connections
						retval = accept(requestListenFD,(struct sockaddr *)&caddr, &len);

						// int retval = acceptClient(requestListenFD);
						if(retval < 0) {
							continue;
						} else if(retval > MAXFD+2) {
							printf("WARNING: Server limit exceeded\nrequest declined!\n");
							continue;
						}

            // //..........Hardcoding the fds of servers connecting to this server(STARTS)........        
            //             cout<<(int)ntohs(caddr.sin_port)<<endl;
			// 			if(((int)ntohs(caddr.sin_port))==6000){
            //                 server1FD=retval;
            //             }
            //             else if((int)ntohs(caddr.sin_port)==7000){
            //                 server2FD=retval;
            //             }
            //             else if((int)ntohs(caddr.sin_port)==8000){
            //                 server4FD=retval;
            //             }
            // //..........Hardcoding the fds of servers connecting to this server(ENDS)........ 

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
							// printf("recv(%d)=%s\n", readfds_arr[i], buff);	//Output Client Sent Information
						 							
                            if(buff[0]=='-'&&buff[1]=='\t'){
			//...........Tokenizing char array to extract name and port number of client(STARTS).........
								char *token = strtok(buff, "\t"); 
								string tokken_array[4];
								int tokken_array_count=0;
								while (token != NULL) 
								{
									tokken_array[tokken_array_count]=string(token);
									tokken_array_count++;
									// printf("%s\n", token); 
									token = strtok(NULL, "\t"); 
								}
								// cout<<server1FD<<"   "<<readfds_arr[i]<<endl;
								// cout<<tokken_array[0]<<"   "<<tokken_array[0].size()<<""<<tokken_array[2]<<"   "<<tokken_array[2].size()<<endl;
			//...........Tokenizing char array to extract name and port number of client(ENDS).........
			                    // if(readfds_arr[i]==server1FD){//If data is new client data is coming from server 1
								    routingTable[routingTableCount].clientName.replace(0,tokken_array[1].size()-1,tokken_array[1]);
									routingTable[routingTableCount].client_PortNo=stoi(tokken_array[2]);
									routingTable[routingTableCount].next_serverName.replace(0,tokken_array[3].size()-1,tokken_array[3]);
									routingTable[routingTableCount].next_toGoFD=readfds_arr[i];
									for(int it1=0;it1<MAXFD;it1++){
										if(it1==i||readfds_arr[it1]==-1||readfds_arr[it1]==requestListenFD){
											// cout<<readfds_arr[it1]<<"    ";
											continue;
										}
										// cout<<endl<<readfds_arr[it1]<<endl;
										// for(int it2=0;buff[it2]!='\0';it2++){
										// 	cout<<buff[it2];
										// }
										// cout<<endl;
										string tosend2 = "-\t"+routingTable[routingTableCount].clientName+"\t"+tokken_array[2]+"\t"+tokken_array[3];
										char buff2[127];
										strcpy(buff2,tosend2.c_str());
										send(readfds_arr[it1],buff2,sizeof(buff2),0);
									}
									// routingTable[routingTableCount].display();
									routingTableCount++;
									displayRoutingTable(routingTable,routingTableCount);
                                // }
                            }
							else if(buff[0]=='-'&&buff[1]=='-'){
								for(int it1=0;it1<MAXFD;it1++){
								if(it1==i||readfds_arr[it1]==-1||readfds_arr[it1]==requestListenFD){
									// cout<<readfds_arr[it1]<<"    ";
									continue;
								}
								send(readfds_arr[it1],buff,sizeof(buff),0);
								}
								char *token = strtok(buff, "\t"); 
								string tokken_array[2];
								int tokken_array_count=0;
								while (token != NULL) 
								{
									tokken_array[tokken_array_count]=string(token);
									tokken_array_count++;
									// printf("%s\n", token); 
									token = strtok(NULL, "\t"); 
								}
								int toDeletePortNo=stoi(tokken_array[1]);
								for(int it1=0;it1<routingTableCount;it1++){
									if(routingTable[it1].client_PortNo==toDeletePortNo){
										routingTable[it1].client_PortNo=0;
										break;
									}
								}
								displayRoutingTable(routingTable,routingTableCount);
							}
							else{
								// cout<<buff<<endl;
								// printf("client msg: %s\n",buff);
								char buffTemp[BUFFSIZE];
								strcpy(buffTemp,buff);
								char *token = strtok(buffTemp, "\t"); 
								string tokken_array[3];
								int tokken_array_count=0;
								while (token != NULL) 
								{
									tokken_array[tokken_array_count]=string(token);
									tokken_array_count++;
									// printf("%s\n", token); 
									token = strtok(NULL, "\t"); 
								}	
								for(int it1=0;it1<routingTableCount;it1++){
									if(routingTable[it1].client_PortNo==0){
										continue;
									}
									if(routingTable[it1].clientName==tokken_array[1]){
										// cout<<routingTable[it1].clientName<<"    "<<tokken_array[1]<<endl<<endl;
										send(routingTable[it1].next_toGoFD,buff,sizeof(buff),0);
									}
								}
							}
              // send(readfds_arr[i],"",0,0);
							//sendto(sockfd, message, MAXLINE, 0, (struct sockaddr*)&servaddr, sizeof(servaddr)); 

						}
					}
				}
			}
			
		}
	}
}

// printf("%s \n", inet_ntoa(cliaddr[0].sin_addr));
// printf("%d \n", (int)ntohs(cliaddr[0].sin_port));
