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
#include <time.h>
#include <fcntl.h>
#include <locale>
#include <fstream>

using namespace std;

#define PORT 5000
#define BACKLOG 10

string HTM = "Content-Type: text/html\r\n";
string HTML = "Content-Type: text/html\r\n";
string JPEG = "Content-Type: image/jpeg\r\n";
string JPG = "Content-Type: image/jpeg\r\n";
string GIF = "Content-Type: image/gif\r\n";
string TXT = "Content-Type: text/plain\r\n";
string BINARY = "Content-Type: application/octet-stream\r\n";
string PDF = "Content-Type: application/pdf\r\n";
string MP4 = "Content-Type: video/mp4\r\n";
string PNG = "Content-Type: image/png\r\n";
string CSS = "Content-Type: text/CSS\r\n";
string WMV = "Content-Type: video/x-ms-wmv\r\n";
string WAV = "Content-Type: audio/x-wav\r\n";
string AAC = "Content-Type: audio/x-aac\r\n";
string WMA = "Content-Type: audio/x-ms-wma\r\n";

string STATUS_ERROR = "HTTP/1.1 404 Not Found\r\n" + HTML +
                      "Content-length: 327\r\n" + "\r\n" + "<!doctype HTML>\n\
<html>\n<head><title> 404 File Not Found</title></head>\n\n\
<body><h1> 404 File NOT Found.</h1><p> The file that you \
requested has unfortunately not been found or \
could not be sent. Please make sure that you are typing it correctly (\
including the spaces, capital / lowecase letters, \
and more!).</p></body>\n\
</html>\n ";
string STATUS_OK = "HTTP/1.1 200 OK\r\n";

/**
 * This method throws the perror and exits the program
 * @param s A string that is the error message
 **/
void throwError(string s) {
  perror(s.c_str());
  exit(1);
}

/**
 * This method parses the HTML request for a file. We MUST be able to handle
 *files with spaces.
 * @param buffer A character array
 * @return string A string representing the file that was found, or an empty
 *string if no file was found.
 **/
string parseFileName(char *buffer);

/**
 * This method will write the return response back to the browser (and show the
 *ERROR code or the
 * correct file/image that needs to be shown).
 * Look into chapter 2 for writing proper HTTP responses (apparently people get
 *points off here!)
 * @param new_fd  The file descriptor of the connected socket
 **/
void writeResponse(int new_fd);

/**
 * This method will get the file type of the requested file.
 * @param file The string which contains the full file name
 * @return string The filetype of the requested file
 **/
string parseFileType(string file);

/**
 * This method will reap zombie processes (signal handler for it)
 * @param sig   The signal for the signal handler
 **/
void handle_sigchild(int sig) {
  while (waitpid((pid_t)(-1), 0, WNOHANG) > 0)
    ;
  fprintf(stderr,
          "Child exited successfully with code %d. Reaped child process.\n",
          sig);
}

int main() {
  int sockfd, new_fd;
  struct sockaddr_in my_addr;
  struct sockaddr_in their_addr;
  socklen_t sin_size;

  // Initiate the connection:
  if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    throwError("socket");
  }

  memset((char *)&my_addr, 0, sizeof(my_addr));

  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(PORT);
  my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) < 0) {
    throwError("bind");
  }

  if (listen(sockfd, BACKLOG) < 0) {
    throwError("listen");
  }

  sin_size = sizeof(struct sockaddr_in);

  // Set up signal handler:
  struct sigaction sa;
  sa.sa_handler = &handle_sigchild;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
  if (sigaction(SIGCHLD, &sa, 0) == -1) {
    throwError("Sig action");
  }

  // Start listening in on connections:
  while (1) {
    if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) <
        0) {
      throwError("accept");
    }

    pid_t pid = fork();

    if (pid == 0) {
      // child process
      close(sockfd);
      writeResponse(new_fd);
      close(new_fd);
      exit(0);
    } else if (pid > 0) {
      // parent process
      close(new_fd);
      continue;
    } else {
      throwError("Fork failed, closing the program");
    }
  }
  return 0;
}

void writeResponse(int new_fd) {
  int n, file_fd;
  struct stat fileInfo;
  time_t t;
  struct tm *timeTm, *modifyTm;
  char currTime[100];
  char modifyTime[100];
  char *fileBuffer;
  char buffer[8192]; // 8192 is usually the largest request size we have to
                     // handle
  memset(buffer, 0, 8192);
  n = read(new_fd, buffer, 8192);
  if (n < 0)
    perror("Error reading from the socket");

  cout << buffer << endl; // Print out the request for the user

  string fileName = parseFileName(buffer);

  /// sometimes we get random requests that don't actually have any input
  /// headers
  if (fileName == "") {
    write(new_fd, STATUS_ERROR.c_str(), STATUS_ERROR.length());
    return;
  }

  // Change %20 to white spaces in fileName if it exists
  for (string::size_type i = 0;
       (i = fileName.find("%20", i)) != string::npos;) {
    fileName.replace(i, 3, " ");
    i += 1;
  }

  // Open file and stat it
  file_fd = open(fileName.c_str(), O_RDONLY);
  if (file_fd < 0) {
    write(new_fd, STATUS_ERROR.c_str(), STATUS_ERROR.length());
    fprintf(stderr, "Could not open file\n");
    return;
  }
  if (fstat(file_fd, &fileInfo) < 0) {
    throwError("bad file");
  }

  // Read in the file and store it into a buffer
  ifstream inFile;
  inFile.open(fileName.c_str(), ios::in | ios::binary | ios::ate);
  streampos fs;
  if (inFile.is_open()) {
    fs = inFile.tellg();
    fileBuffer = new char[(long long)(fs) + 1];
    inFile.seekg(0, ios::beg);
    inFile.read(fileBuffer, fs);
    inFile.close();
  }

  // Create the date strings
  t = time(NULL);
  timeTm = gmtime(&t);
  strftime(currTime, sizeof(currTime), "Date: %a, %d %b %G %T GMT",
           timeTm); // Sun, 26 Sep 2010 20:09:20 GMT\r\n format
  modifyTm = gmtime(&fileInfo.st_mtime);
  strftime(modifyTime, sizeof(modifyTime), "Last-Modified: %a, %d %b %G %T GMT",
           modifyTm);

  // Other required strings for the response header
  string responseStatus = STATUS_OK;
  string date(currTime);
  date += "\r\n";
  string server = "Server: Gandhi-Jasapara Server\r\n";
  string lastModified(modifyTime);
  lastModified += "\r\n";
  long long fileLength = fileInfo.st_size;
  char cLength[1000];
  sprintf(cLength, "Content-length: %lld\r\n", fileLength);
  string contentLength(cLength);
  string closeConnection = "Connection: close\r\n";
  string contentDisposition = "Content-Disposition: inline\r\n";
  string contentType = parseFileType(fileName);

  string respHeader = responseStatus + date + server + lastModified +
                      contentLength + contentType + contentDisposition +
                      closeConnection + "\r\n";

  cout << respHeader << endl; // Print out the resp header to show the user

  // Write the response header and body out to the server, and close the
  // connection and clear heap allocated space
  write(new_fd, respHeader.c_str(), respHeader.length());
  write(new_fd, fileBuffer, fileLength);
  close(file_fd);
  delete[] fileBuffer;
}

string parseFileName(char *buffer) {
  string request(buffer);
  if (request == "") {
    return "";
  }
  size_t found = request.find(" ");
  size_t found2 = request.find(" HTTP/1.1\r\n", found + 1, 10);
  string temp = found2 - found - 2 > 0
                    ? request.substr(found + 2, found2 - found - 2)
                    : "";
  if (temp == "") {
    return "";
  } else { // convert the ending to lower case
    size_t foundPeriod = temp.find(".");
    if (foundPeriod == string::npos) {
      return temp;
    } else {
      string temp2 = temp.substr(0, foundPeriod);
      for (unsigned int i = foundPeriod; i < temp.length(); i++) {
        temp2 += tolower(temp[i]);
      }
      return temp2;
    }
  }
}

string parseFileType(string inputFile) {
  string file = "";

  // Make the file case insensitive
  for (unsigned int i = 0; i < inputFile.length(); i++) {
    file += tolower(inputFile[i]);
  }

  if (file.find(".html") != string::npos) {
    return HTML;
  } else if (file.find(".htm") != string::npos) {
    return HTML;
  } else if (file.find(".gif") != string::npos) {
    return GIF;
  } else if (file.find(".jpeg") != string::npos) {
    return JPEG;
  } else if (file.find(".jpg") != string::npos) {
    return JPG;
  } else if (file.find(".txt") != string::npos) {
    return TXT;
  } else if (file.find(".pdf") != string::npos) {
    return PDF;
  } else if (file.find(".mp4") != string::npos) {
    return MP4;
  } else if (file.find(".png") != string::npos) {
    return PNG;
  } else if (file.find(".css") != string::npos) {
    return CSS;
  } else if (file.find(".wmv") != string::npos) {
    return WMV;
  } else if (file.find(".wav") != string::npos) {
    return WAV;
  } else if (file.find(".aac") != string::npos) {
    return AAC;
  } else if (file.find(".wma") != string::npos) {
    return WMA;
  }

  return BINARY; // by default we will assume everything else is a text file
}