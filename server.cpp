// Sahil Gandhi and Arpit Jasapara
// CS 118 Winter 2018
// Project 1
// This is the server code for the first project.

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <signal.h>
#include <string>

using namespace std;

#define PORT 5000
#define BACKLOG 5

void throwError(string s){
  perror(s.c_str());
  exit(1);
}

int main(){
  int sockfd, new_fd;
  struct sockaddr_in my_addr;
  struct sockaddr_in their_addr;
  socklen_t sin_size;

  if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
    throwError("socket");
  }
  
  memset((char *) &my_addr, 0, sizeof(my_addr));

  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(PORT);
  my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) < 0){
    throwError("bind");
  }
  
  if (listen(sockfd, BACKLOG) < 0){
    throwError("listen");
  }

  while(1){
    sin_size = sizeof(struct sockaddr_in);
    if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size)) < 0){
      perror("accept");
      
      int n;
      char buffer[256];

      memset(buffer, 0, 256);

      n = read(new_fd, buffer, 255);
      if (n < 0)
        perror("Error reading from the socket");
      for (int i = 0; i < 255; i++){
        cout << buffer[i];
      }
      cout << endl;

      close(new_fd);
      close(sockfd);
      return 0;
    }


  }
}
