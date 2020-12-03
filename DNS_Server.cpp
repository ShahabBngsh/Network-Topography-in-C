#include "libraries.h"
#include "RoutingTable.h"
using namespace std;


/*THANKS to jwhitlock from
https://stackoverflow.com/questions/717572/how-do-you-do-non-blocking-console-i-o-on-linux-in-c
*/
bool inputAvailable() {
  struct timeval tv{1, uSec*0};
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
  return (FD_ISSET(0, &fds));
}

void readfunc(int readfds_arr[], fd_set readfds, int sockfd_s4){
	FD_ZERO(&readfds);//Clear the readfds array to 0
		int maxfd = -1;
		//For loop finds the maximum subscript for the ready event in the fds array
		for(int i=0; i<MAXFD_C; i++)	{
			if(readfds_arr[i] == -1) {
				continue;
			}
			FD_SET(readfds_arr[i],  &readfds);
			if(readfds_arr[i] > maxfd) {
				maxfd = readfds_arr[i];
			}
		}

		struct timeval tv = {1, 0};
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
			for(int i=0; i < MAXFD_C; ++i)	{
				if(readfds_arr[i]==-1) {	//If readfds_arr[i]==-1, the event is not ready
					continue;
				}//Determine if the event corresponding to the file descriptor is ready
				if(FD_ISSET(readfds_arr[i], &readfds)) {
					// //accept if a new client requests a connection
					char buff[BUFFSIZE]={0};
					int res = recv(readfds_arr[i], buff, BUFFSIZE, 0);
					char *token = strtok(buff, "\t"); 
					string tokens[3];
					int tokCounter = 0;
					while (token != NULL) {
						tokens[tokCounter] = string(token);
						tokCounter++;
						token = strtok(NULL, "\t"); 
					}
					//remove extra char from tokens[2] at the end. 
					string webName = tokens[2].substr(0, tokens[2].length()-1);
					string segment = "dns\t" + tokens[0] + "\t"; //construct data segment for reply
					string ip = checkWebsite("websites.txt", webName);
					if (ip == "") {//if ip not found it'll return empty string
						segment += "Error 404: website not found";
						send(sockfd_s4, segment.c_str(), segment.length(), 0);
					} else {
						segment += ip;
						send(sockfd_s4, segment.c_str(), segment.length(), 0);
					}
					//cout << tokens[0] << "> " << tokens[2] << endl;
				}
			}
		
		}
}



int main() {

	// string str = checkWebsite("websites.txt", "www.reddit.com");
	// cout << str << endl;

	char buff[BUFFSIZE] = {'\0'};
	int sockfd_s4 = connectSock2Port(localhost, S4PORTNO);

	char firstNewClinetMsg[20]=".dns";
	sendto(sockfd_s4, firstNewClinetMsg, strlen(firstNewClinetMsg), 0, 0, 0);
	
	fd_set readfds_set;
	//Define fds array
	int readfds_arr[MAXFD_C];
	for(int i=0; i<MAXFD_C; ++i) {
		readfds_arr[i]  = -1;
	}
	//Add a file descriptor to the fds array
  fds_init(readfds_arr,  sockfd_s4);

	while(1) {

		while (!inputAvailable()) {
      readfunc(readfds_arr, readfds_set, sockfd_s4);
      //sleep(1);
    }
		read(STDIN_FILENO, buff, sizeof(buff));
		string str = buff;
		str = "c4\tc2\t" + string(buff);
		send(sockfd_s4, str.c_str(), str.length(), 0);
		memset(buff, 0, BUFFSIZE);

	}
	close(sockfd_s4);
}


