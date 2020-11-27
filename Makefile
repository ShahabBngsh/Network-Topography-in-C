
all: server1 client1

server1: server.cpp
	g++ server.cpp -o s1
	g++ server.cpp -o s2

client1: client.cpp
	g++ client.cpp -o c1
	g++ client.cpp -o c2
	g++ client.cpp -o c3

clean:
	rm -rf s1 s2 c1 c2
