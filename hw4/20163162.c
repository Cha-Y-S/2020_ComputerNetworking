/*
Student ID : 20163162
Name : Yoonsung Cha
*/
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 100

void error_handling(char *message);
void display();

#define MAXCLIENT (8)
int clienttcpSocket[MAXCLIENT];	// max 8 clients


int main(int argc, char **argv) {
	
  int tcpServ_sock;
  
  struct sockaddr_in tcpServer_addr;
  struct sockaddr_in tcpClient_addr;
  struct sockaddr_in newTcp_addr;
  
  // initialize the client socket descriptors
  for(int i = 0; i < MAXCLIENT; i++) {
  	clienttcpSocket[i] = -1;;	// max 8 clients
  }

  int slot;
  socklen_t clnt_len;
    
  fd_set reads, temps;
  int fd_max;

  
  char str[BUFSIZE];
  int option = 2;

  char *tcpport = NULL;

  if(argc != 2) {
    printf("Usage : %s <tcpport> \n", argv[0]);
    exit(1);
  }

  tcpport = argv[1];

  display();
	
  tcpServ_sock = socket(PF_INET, SOCK_STREAM, 0);
  if(tcpServ_sock == -1)
	  error_handling("socket() error");
  
  memset(&tcpServer_addr, 0 , sizeof(tcpServer_addr));
  tcpServer_addr.sin_family = AF_INET;
  tcpServer_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  tcpServer_addr.sin_port = htons(atoi(tcpport));

  setsockopt(tcpServ_sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&option, sizeof(int));
  if(bind(tcpServ_sock, (struct sockaddr *) &tcpServer_addr, sizeof(tcpServer_addr)) == -1 )
	  error_handling("bind() error");

			
  if(listen(tcpServ_sock, 5)==-1)
    error_handling("listen() error");
  
  FD_ZERO(&reads);
  FD_SET(tcpServ_sock, &reads);
  fd_max = tcpServ_sock;
  
  int is_full = 0;

  while(1) {
    int nfound = 0;

    temps = reads;
    
    nfound = select(fd_max+1, &temps, 0, 0, NULL);
	
	  if(FD_ISSET(tcpServ_sock, &temps)) {
		  // Input from the client for new connection
		  // This request will arrive after UDP sent
      // connect request from a peer -> accept(TODO)
		  FD_CLR(tcpServ_sock, &temps);
	    // NEED TO IMPLEMENT HERE
      clnt_len = sizeof(tcpClient_addr);
		  int index = -1;
      while(clienttcpSocket[++index] != -1){
        if(index == MAXCLIENT){
          is_full = 1;
          break;
        }
      }
      if(is_full){
        int tmp = accept(tcpServ_sock, (struct sockaddr *)&tcpClient_addr, &clnt_len);
        printf("server is full!\n");
        break;
      } else{
        clienttcpSocket[index] = accept(tcpServ_sock, (struct sockaddr *)&tcpClient_addr, &clnt_len);
        if (clienttcpSocket[index] < 0) {
			    perror("accept");
			    continue;
		    }
		    printf("connection from host %s, port %d, socket %d\n",
			    inet_ntoa(tcpClient_addr.sin_addr), ntohs(tcpClient_addr.sin_port),clienttcpSocket[index]);
		    FD_SET(clienttcpSocket[index], &reads);
	      fd_max = clienttcpSocket[index];
      }
	  } else {
		  for(int i = 0; i < MAXCLIENT; i++) {
		    if (FD_ISSET(clienttcpSocket[i], &temps)) {
			    FD_CLR(clienttcpSocket[i], &temps);

  			  /* process the data */
         // forward the message to all the other sockets
         memset(str, 0, sizeof(str));
         int bytesread = read(clienttcpSocket[i], str, BUFSIZE);

          if(bytesread <= 0){
            printf("Connection closed %d\n", clienttcpSocket[i]);
            clienttcpSocket[i] = -1;
          }
			    // NEED TO IMPLEMENT HERE
          for(int j = 0; j < MAXCLIENT; j++){
            if(j == i || clienttcpSocket[j] == -1){
              continue;
            }
            write(clienttcpSocket[j], str, bytesread);
          }
        }
		  }
	  }

  }//while End
}//main End

void display() {
	printf("Student ID : 20163162 \n");
	printf("Name : Yoonsung Cha  \n");
}

void error_handling(char *message) {
  fputs(message, stderr);
  fputc('\n', stderr);
  perror("hw4 error");
  exit(1);
}


