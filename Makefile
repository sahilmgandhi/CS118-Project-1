CC=gcc

default:
	$(CC) -Wall -Wextra -g -o p1_server -I. server.cpp

clean:
	rm -rf p1_server

