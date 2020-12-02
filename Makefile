
all: servers clients

servers: server1.cpp server2.cpp server3.cpp server4.cpp
	g++ server1.cpp -o s1
	g++ server2.cpp -o s2
	g++ server3.cpp -o s3
	g++ server4.cpp -o s4

clients: client1.cpp client2.cpp client4.cpp
	g++ client1.cpp -o c1
	g++ client2.cpp -o c2
	g++ client4.cpp -o c4

clean:
	rm -rf s1 s2 s3 s4 c1 c2 c3 c4
