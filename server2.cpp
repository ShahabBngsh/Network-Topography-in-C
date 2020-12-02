#include "libraries.h"
#include<cstdlib>
#include<stdlib.h>
#include<assert.h>
using namespace std;

//#define MAXFD 6	//Size of fds array
#define PORTNO 7000
#define backlog 3
#define localhost "127.0.0.1"
#define BUFFSIZE 127
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
			// cout<<endl<<it1<<endl;disconnected!
			routingTable[it1].display();
		}
	}
	// cout<<counter<<endl;
	cout<<"***********************************************************\n";
}
//Add a file descriptor to the fds array
// void fds_init(int fds_arr[],int fd) {
// 	for(int i=0; i<MAXFD; ++i) {
// 		if(fds_arr[i]==-1) {
// 			fds_arr[i]=fd;
// 			break;
// 		}
// 	}
// }

// void checkError(const int& ret,const char* s) {
// 	if(ret == -1) {
// 		perror(s);
// 		exit(-1);
// 	}
// }
// int acceptClient(int requestListenFD) {
// 	struct sockaddr_in caddr;
// 	socklen_t len=sizeof(caddr);
// 	//Accept new client connections
// 	return accept(requestListenFD,(struct sockaddr *)&caddr, &len);
// }
struct clientPorts{
	int portNo=0;
	bool available=true;
};

int main() {
	RoutingTableRow routingTable[10];
	clientPorts ports_array[10];
	int routingTableCount=0;


//.......DEFINING OWN ADRESS AND OPENING SOCK TO ACCEPT CONNECTION REQUESTS (STARTS).................

	int requestListenFD=socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in saddr;
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(PORTNO);
	saddr.sin_addr.s_addr=inet_addr(localhost);

	int res2=bind(requestListenFD,(struct sockaddr*)&saddr,sizeof(saddr));
	checkError(res2, "Error bind");
	
	//Create listening queue
	listen(requestListenFD, backlog);

//.......DEFINING OWN ADRESS AND OPENING SOCK TO ACCEPT CONNECTION REQUESTS (ENDS).................	

//..........CONNECTION REQUEST WITH SERVER3 CODE (STARTS).......... 

	int forServer3sockfd=socket(AF_INET,SOCK_STREAM,0);
	checkError(forServer3sockfd, "Error socket");


    struct sockaddr_in server3_addr;
	memset(&server3_addr,0,sizeof(server3_addr));
	server3_addr.sin_family = AF_INET;
	server3_addr.sin_port = htons(10000);
	server3_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int res1 = connect(forServer3sockfd,(struct sockaddr*)&server3_addr,sizeof(server3_addr));
    // assert(res1 != -1);

//..........CONNECTION REQUEST WITH SERVER3 CODE (ENDS).......... 




	//Define fdset collection
	fd_set readfds, writefds;
	
	//Define fds array
	int readfds_arr[MAXFD], writefds_arr[MAXFD];
	for(int i=0; i<MAXFD; ++i) {
		readfds_arr[i]  = -1;
		writefds_arr[i] = -1;
	}
	
	//Add a file descriptor to the fds array

	fds_init(readfds_arr,  requestListenFD);
	fds_init(writefds_arr, requestListenFD);
	fds_init(readfds_arr,  forServer3sockfd);
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
					// cout<<"Entering Is set\n";
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
						// printf("----------------------------------\n");
						// printf("%s\n",inet_ntoa(caddr.sin_addr));
						// printf("%d\n",(int)ntohs(caddr.sin_port));
						// printf("----------------------------------\n");
						ports_array[retval].portNo=(int)ntohs(caddr.sin_port);
						// ports_array[retval].available=false;
						printf("accept c=%d\n",retval);
						fds_init(readfds_arr, retval);//Add the connection socket to the readfds_arr array
					} else {  //Receive data recv when an existing client sends data
						socklen_t len=100;
						char buff[BUFFSIZE]={0};
					// 	int res = recvfrom(readfds_arr[i], buff, BUFFSIZE, 0, (struct sockaddr*)&client_addr2,&len); //receive message from server 
					// 	// int result = getpeername ( readfds_arr[i] , (struct sockaddr*) &addr , &len );
					// 	printf("%d\n",readfds_arr[i]);
					// 	// if(result==0){
					// 	// 	printf("%s\n",inet_ntoa(addr.sin_addr));
					// 	// 	printf("%d\n",(int)ntohs(addr.sin_port));
					// 	// }
					// 	printf("%s\n",inet_ntoa(client_addr.sin_addr));
					// 	printf("%d\n",(int)ntohs(client_addr.sin_port));
					// //	buf[n] = '\0'; 

						int res=recv(readfds_arr[i],buff,BUFFSIZE,0);
						// cout<<buff<<endl;
						if(res <= 0) {
							close(readfds_arr[i]);
							for(int it1=0;it1<routingTableCount;it1++){
								if(routingTable[it1].client_PortNo==ports_array[readfds_arr[i]].portNo){
									string deleteMsg="--	"+to_string(routingTable[it1].client_PortNo);
									char todelete[deleteMsg.size()+1];
									strcpy(todelete,deleteMsg.c_str());
									send(forServer3sockfd,todelete,sizeof(todelete),0);
									routingTable[it1].client_PortNo=0;
									ports_array[readfds_arr[i]].portNo=0;
									ports_array[readfds_arr[i]].available=true;
									displayRoutingTable(routingTable,routingTableCount);
									// routingTable[it1].display();
									break;
								}
							}

							readfds_arr[i]=-1;

							printf("client disconnected!\n");
						} else {
							// printf("recv(%d)=%s\n", readfds_arr[i], buff);	//Output Client Sent Information
							string str=string(buff);
							// char newEntry[100]="New entry";
							
							if(buff[0]=='.'){
								if(ports_array[readfds_arr[i]].available){
								string str2="self";
								// printf("recv(%d)=%s\n", readfds_arr[i], buff);	//Output Client Sent Information
								// printf("%s\n",newEntry);
								// printf("%d\n",((int)ntohs(client_addr.sin_port)));
								// routingTable[routingTableCount].client_PortNo=((int)ntohs(client_addr.sin_port));
								string substr=str.substr(1,str.size()-1);
								routingTable[routingTableCount].clientName.replace(0,substr.size()-1,substr);
								routingTable[routingTableCount].client_PortNo=ports_array[readfds_arr[i]].portNo;
								ports_array[readfds_arr[i]].available=false;
								routingTable[routingTableCount].next_serverName.replace(0,5,str2);
								routingTable[routingTableCount].next_toGoFD=readfds_arr[i];
								// routingTable[routingTableCount].display();
								string tosend="-	"+routingTable[routingTableCount].clientName+"	"+to_string(routingTable[routingTableCount].client_PortNo)+"	server2";
								// char msg[tosend.size()+1]="-	usama	12345	server1";
								char msg[tosend.size()+1];
								strcpy(msg,tosend.c_str());
								send(forServer3sockfd,msg,sizeof(msg),0);	
								routingTableCount++;								
								displayRoutingTable(routingTable,routingTableCount);
								}
								else{
									printf("client msg: %s\n",buff);									
								}
							}
							else if(buff[0]=='-'&&buff[1]=='\t'){
								char *token = strtok(buff, "\t"); 
								string tokken_array[4];
								int tokken_array_count=0;
								while (token != NULL) 
								{
									tokken_array[tokken_array_count]=string(token);
									tokken_array_count++;
									printf("%s\n", token); 
									token = strtok(NULL, "\t"); 
								}
								routingTable[routingTableCount].clientName.replace(0,tokken_array[1].size()-1,tokken_array[1]);
								routingTable[routingTableCount].client_PortNo=stoi(tokken_array[2]);
								routingTable[routingTableCount].next_serverName.replace(0,tokken_array[3].size()-1,tokken_array[3]);
								routingTable[routingTableCount].next_toGoFD=readfds_arr[i];
								routingTableCount++;								
								displayRoutingTable(routingTable,routingTableCount);
								printf("client msg: %s\n",buff);
							}
							else if(buff[0]=='-'&&buff[1]=='-'){
								char *token = strtok(buff, "\t"); 
								string tokken_array[2];
								int tokken_array_count=0;
								while (token != NULL) 
								{
									tokken_array[tokken_array_count]=string(token);
									tokken_array_count++;
									printf("%s\n", token); 
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
					if(writefds_arr[i]==requestListenFD) {
						//Accept new client connections
						int retval = acceptClient(requestListenFD);
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
