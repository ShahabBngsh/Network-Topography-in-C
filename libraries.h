#ifndef LIBRARIES_H
#define LIBRARIES_H

//all required libraries
#include<stdio.h>
#include<cstdlib>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<string.h>

#include <string>
#include <iostream>
#include <fstream>
#include <bits/stdc++.h> 

#define localhost "127.0.0.1"
//assigned port numbers
#define S1PORTNO 6000 //server1 portno
#define S2PORTNO 7000 //server2 portno
#define S3PORTNO 10000 //server3(main server)
#define S4PORTNO 8000 //server4 

#define backlog 5
#define MAXFD 10	//Size of fds array
// maxfd for clients, as client only connect with
//its dedicated server that's why value is 1
# define MAXFD_C 1 
#define BUFFSIZE 127
#define uSec 500000 //microseconds used in timeval struct; 1sec = 1000000 usec

#endif