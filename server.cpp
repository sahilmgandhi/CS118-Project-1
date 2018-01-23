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
#define BACKLOG 10

string HTML = "Content-Type: text/html\r\n"; 
string JPEG = "Content-Type: image/jpeg\r\n"; 
string JPG = "Content-Type: image/jpeg\r\n"; 

/**
 * This method throws the perror and exits the program
 * @param s A string that is the error message
 **/
void throwError(string s){
  perror(s.c_str());
  exit(1);
}

/**
 * This method parses the HTML request for a file. We MUST be able to handle files with spaces.
 * @param buffer A character array 
 * @return string A string representing the file that was found, or an empty string if no file was found. 
 **/
string parseFileName(char* buffer){
  return "a"; // placeholder
}

/**
 * This method will write the return response back to the browser (and show the ERROR code or the 
 * correct file/image that needs to be shown).
 * Look into chapter 2 for writing proper HTTP responses (apparently people get points off here!)
 * @param ?? What might we need?
 **/
void writeResponse();

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

  sin_size = sizeof(struct sockaddr_in);

  while (1){
    if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size)) < 0){
      throwError("accept");
    }

    // fork here? How do we keep this running for a long time while keeping the server open for multiple requests from the browser?

    int n;
    char buffer[8192];    // 8192 is usually the largest size that we may have to handle/ 

    memset(buffer, 0, 8192);

    n = read(new_fd, buffer, 8191);   // Is there any way to handle reading in chunks (say multiple chunks of 8192?)
    if (n < 0)
      perror("Error reading from the socket");
    cout << buffer << endl;
    close(new_fd);
  }
  close(sockfd);
  return 0;
}
