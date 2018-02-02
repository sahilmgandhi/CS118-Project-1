CC=g++
UID=704598105

default:
	$(CC) -Wall -Wextra -g -o p1_server -I. server.cpp

dist:
	tar -cvzf $(UID).tar.gz server.cpp README report.pdf Makefile

clean:
	rm -rf p1_server
	rm $(UID).tar.gz

