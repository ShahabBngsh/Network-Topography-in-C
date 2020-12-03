// #include "libraries.h"
#include "RoutingTable.h"
using namespace std;

struct DNS_Info{
	string URL="";
	string receiver_client="";
	bool slotBooked=false;
};

void addWebsite(const string& filename, const string& line) {
	ofstream outfile;
	//append to the file, instead of overwrite
  outfile.open(filename, std::ios_base::app);
  outfile << line; 
}

int main() {
	DNS_Info dns_array[7];
	int dns_counter=0;
	RTRow rt[10];
	int rtCounter=0;
	int server1FD = -1, server2FD = -1, server4FD = -1;

	int reqListenFD = bindSock2Port(localhost, S3PORTNO);	
	//Define fdset collection
	fd_set readfds;
	
	//Define fds array
	int readfds_arr[MAXFD];
	for(int i=0; i<MAXFD; ++i) {
		readfds_arr[i]  = -1;
	}
	
	//Add a file descriptor to the fds array
  fds_init(readfds_arr,  reqListenFD);

	while(1) {
		FD_ZERO(&readfds);//Clear the readfds array to 0
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

		struct timeval tv = {15, 0};
		//select system call, where we only focus on read events
		int retval = select(maxfd+1, &readfds, NULL, NULL, &tv);
		if(retval == -1)	{ //fail
			perror("Error select()");
		}
		else if(retval == 0) { //Timeout, meaning no file descriptor returned
			//printf("time out\n");
		}
		else { //Ready event generation
			//loop through each readfd, to see if it has something to read from
			for(int i=0; i < MAXFD; ++i)	{
				if(readfds_arr[i]==-1) {	//If readfds_arr[i]==-1, the event is not ready
					continue;
				}//Determine if the event corresponding to the file descriptor is ready
				if(FD_ISSET(readfds_arr[i], &readfds)) {
					//accept if a new client requests a connection
					if(readfds_arr[i]==reqListenFD) {
						//Accept new client connections
            struct sockaddr_in caddr;
						socklen_t len=sizeof(caddr);
						//Accept new client connections
						retval = accept(reqListenFD,(struct sockaddr *)&caddr, &len);

						// int retval = acceptClient(reqListenFD);
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
						int res = recv(readfds_arr[i], buff, BUFFSIZE, 0);
						if(res <= 0) {
							close(readfds_arr[i]);
							readfds_arr[i] = -1;
							printf("Server %d down!!\n", i);
						} else {
							if(buff[0]=='-' && buff[1] == '\t') {
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
								for(int it1 = 0; it1 < MAXFD; it1++){
									if(it1 == i || readfds_arr[it1] == -1 || readfds_arr[it1] == reqListenFD) {
										continue;
									}
									string tosend2 = "-\t" + rt[rtCounter].cName + "\t" + tokArr[2] + "\t" + tokArr[3];
									char buff2[BUFFSIZE];
									strcpy(buff2, tosend2.c_str());
									send(readfds_arr[it1], buff2, sizeof(buff2), 0);
								}
								rtCounter++;
								dispRT(rt, rtCounter);
			                } else if(buff[0] == '-' && buff[1] == '-') {
								for(int it1 = 0; it1 < MAXFD; it1++) {
									if(it1 == i || readfds_arr[it1] == -1 || readfds_arr[it1] == reqListenFD) {
										continue;
									}
									send(readfds_arr[it1], buff, sizeof(buff), 0);
								}
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
								dispRT(rt, rtCounter);
							} else {
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
								if(tokArr[0]=="dns"){
									for(int it2=0;it2<7;it2++){
										if(dns_array[it2].slotBooked==false){
											continue;
										}
										if(dns_array[it2].receiver_client==tokArr[1]){
											string webName = dns_array[it2].URL.substr(0, dns_array[it2].URL.length()-1);									
											string toWriteInProxyDNS=webName+" "+tokArr[2];
											if(tokArr[2] != "Error 404: website not found"){;
												writeWebsite("proxy.txt",toWriteInProxyDNS);
											}
											dns_array[it2].URL.replace(0,dns_array[it2].URL.size(),"");
											dns_array[it2].receiver_client.replace(0,dns_array[it2].receiver_client.size(),"");
											dns_array[it2].slotBooked=false;
											break;
										}
									}
								}								
								for(int it1 = 0; it1 < rtCounter; it1++) {
									if(rt[it1].cPortNo == 0) {
										continue;
									}
									if(rt[it1].cName == tokArr[1]) {
										if(rt[it1].cName=="dns"){
										string webName = tokArr[2].substr(0, tokArr[2].length()-1);
										string ip = checkWebsite("proxy.txt", webName);
											if(ip==""){
												for(int it2=0;it2<7;it2++){
													if(dns_array[dns_counter].slotBooked==true){
														continue;
													}
													dns_array[it2].URL.replace(0,tokArr[2].size()-1,tokArr[2]);
													dns_array[it2].receiver_client.replace(0,tokArr[0].size()-1,tokArr[0]);
													dns_array[it2].slotBooked=true;
													send(rt[it1].nextFD, buff, sizeof(buff),0);
													break;
												}
											}
											else{
												string toSendIp="proxy "+tokArr[1]+"\t"+tokArr[0]+"\t"+ip;
												char ipReply[127];
												strcpy(ipReply,toSendIp.c_str());
												send(readfds_arr[i],ipReply,sizeof(ipReply),0);
											}
										}
										else{
											send(rt[it1].nextFD, buff, sizeof(buff),0);
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
