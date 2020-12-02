#include "libraries.h"
using namespace std;

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

struct RTRow {
	string cName = "";
	int cPortNo = 0;
	int nextFD = -1;
	string nextSName = "";

	void display()const{
		cout<< "cName: " << cName << "  Clinet_PortNo: " << cPortNo 
				<< "  NextServerName :" << nextSName << "  Next_toGoFd: "
				<< nextFD << endl;
	}
};

void dispRT(RTRow* rt, int RTcounter){
	// system("clear");
	cout<<"***********************************************************\n";
	int counter = 0;
	for(int it1 = 0; it1 < RTcounter; it1++) {
		if(rt[it1].cPortNo!=0){
			counter++;
			rt[it1].display();
		}
	}
	cout<<"***********************************************************\n";
}

struct cPort{
	int portNo=0;
	bool available=true;
};

/*open a socket, bind that socket to portNo
return file discriptor
*/
int bindSock2Port(const char* ip, const int& portNo) {
	int reqListenFD = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in saddr;
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port=htons(portNo);
	saddr.sin_addr.s_addr=inet_addr(ip);

	int retBind=bind(reqListenFD,(struct sockaddr*)&saddr,sizeof(saddr));
	checkError(retBind, "Error bind");

	//Create listening queue
	listen(reqListenFD, backlog);
  return reqListenFD;
}

/*create socket, create obj of sockaddr_in, assign ip, portNo, then connect to the sever
return file discriptor
*/
int connectSock2Port(const char* ip, const int& portNo) {
	int sockfd=socket(AF_INET,SOCK_STREAM,0);
	checkError(sockfd, "Error socket");
  struct sockaddr_in server;
	memset(&server,0,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(portNo);
	server.sin_addr.s_addr = inet_addr(ip);

	int retval = connect(sockfd, (struct sockaddr*)&server, sizeof(server));
	checkError(retval, "Error: connection with Server3 ");
  return sockfd;
}
