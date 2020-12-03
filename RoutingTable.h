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

	void display(const char& start, const char& end)const{
		cout << "|" << cName << "\t " << cPortNo << "\t\t" 
		<< nextSName << "\t\t" << nextFD << "\t|" << endl;
	}
};
void dispChar(const char& start, const char& middle, const char& end, const int& n) {
	cout << start;
	for (int i = 0; i < n; i++) {
		cout << middle ;
	}
	cout << end;
	
}
void dispRT(RTRow* rt, int RTcounter){
	int n = 47;
	dispChar('+', '_', '+', n);
	cout << "\n|cName\tClientPortNo\tNextServerName\tNextFD  |\n";
	int counter = 0;
	for(int it1 = 0; it1 < RTcounter; it1++) {
		if(rt[it1].cPortNo!=0){
			counter++;
			rt[it1].display('+', '+');
		}
	}
	dispChar('+', '-', '+', n);
	cout << endl;
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

string checkWebsite(const string& filename, const string& website) {
	string line = "", web, ip;
	bool found = false;
	ifstream file(filename);

	if (file.is_open()) {
		while( getline(file, line)) {
			line += '\n';
			stringstream ss(line);
			getline(ss, web, ' ');
			if (web == website)	{
				getline(ss, ip, '\n');
				found = true;
				break;
			}
		}
		file.close();
		if(found) {
			return ip;
		} else {
			return "";
		}
	} else {
		cout << "file not found\n";
		return "";
	}
}

