#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<cstring>
#include<assert.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<iostream>
using namespace std;
int main() {
	int sockfd = socket(AF_INET,SOCK_STREAM,0);	
//	assert(sockfd != -1 );

	//Set Address Information
	struct sockaddr_in saddr;
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(7000);
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//Link to server
	int res = connect(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
	assert(res != -1);
	char firstNewClinetMsg[20]=".client2";
	sendto(sockfd, firstNewClinetMsg, strlen(firstNewClinetMsg), 0, (struct sockaddr*)&saddr, sizeof(saddr));
	while(1) {
		char buff[128];// = "Hello client 4 from client 1";
		printf("Please Input:");
		cin.getline(buff,128,'\n');
		string temp=buff;
		// fgets(buff,128,stdin);
		// if(strncmp(buff,"end",3) ==0 )
		// {
		// 	break;
		// }
		string msgToSendTemp="client2\tclient4\t"+temp;
		char msgToSend[150];
		strcpy(msgToSend,msgToSendTemp.c_str());
		// sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr*)&saddr, sizeof(saddr)); 
		send(sockfd,msgToSend,strlen(msgToSend),0);
		memset(msgToSend,0,128);
		// sleep(100);
		// while(1){

		// }
		//recv(sockfd,buff,127,0);
		// printf("RecvBuff:%s\n",buff);
    // printf("\n");
	}
	close(sockfd);
}


