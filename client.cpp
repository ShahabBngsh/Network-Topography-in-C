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
	saddr.sin_port = htons(6000);
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	//Link to server
	int res = connect(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
	assert(res != -1);
	char firstNewClinetMsg[20]=".client1";
	sendto(sockfd, firstNewClinetMsg, strlen(firstNewClinetMsg), 0, (struct sockaddr*)&saddr, sizeof(saddr));
	while(1) {
		char buff[128] ={0};
		// printf("Please Input:");
		// cin.getline(buff,128,'\n');
		// fgets(buff,128,stdin);
		// if(strncmp(buff,"end",3) ==0 )
		// {
		// 	break;
		// }
		// sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr*)&saddr, sizeof(saddr)); 
		// send(sockfd,buff,strlen(buff),0);
		// memset(buff,0,128);
		recv(sockfd,buff,127,0);
		        char *token = strtok(buff, "\t"); 
        string tokken_array[3];
        int tokken_array_count=0;
        while (token != NULL) 
        {
            tokken_array[tokken_array_count]=string(token);
            tokken_array_count++;
            // printf("%s\n", token); 
            token = strtok(NULL, "\t"); 
        }
    	cout<<"Received Messege: "<<tokken_array[2]<<endl;
	// printf("RecvBuff:%s\n",buff);
    // printf("\n");
	}
	close(sockfd);
}


