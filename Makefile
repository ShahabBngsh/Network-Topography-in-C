
all: server1 client1

server1: main_server.cpp server1.cpp server3.cpp server.cpp server4.cpp
	g++ main_server.cpp -o sm
	g++ server1.cpp -o s1
	g++ server2.cpp -o s2
	g++ server3.cpp -o s3
	g++ server4.cpp -o s4
client1: client1.cpp client.cpp client2.cpp client4.cpp
	g++ client1.cpp -o c1
	g++ client1.cpp -o c2
	g++ client1.cpp -o c3
	g++ client2.cpp -o c_2
	g++ client4.cpp -o c4
	g++ client.cpp -o c

clean:
	rm -rf s1 s2 s3 s4 c1 c2 c3 c_2 c4 c
