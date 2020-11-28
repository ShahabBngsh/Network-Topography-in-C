
all: server1 client1

server1: main_server.cpp server1.cpp
	g++ main_server.cpp -o s1
	g++ server1.cpp -o s2

client1: client1.cpp
	g++ client1.cpp -o c1
	g++ client1.cpp -o c2
	g++ client1.cpp -o c3

clean:
	rm -rf s1 s2 c1 c2 c3
