
all: servers clients

servers: server1.cpp server2.cpp server3.cpp server4.cpp DNS_Server.cpp
	g++ server1.cpp -o s1
	g++ server2.cpp -o s2
	g++ server3.cpp -o s3
	g++ server4.cpp -o s4
	g++ DNS_Server.cpp -o dns

clients: client1.cpp client2.cpp client3.cpp client4.cpp client6.cpp client7.cpp
	g++ client1.cpp -o c1
	g++ client2.cpp -o c2
	g++ client3.cpp -o c3
	g++ client4.cpp -o c4
	g++ client6.cpp -o c6
	g++ client7.cpp -o c7

clean:
	rm -rf s1 s2 s3 s4 dns c1 c2 c3 c4 c6 c7
