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

void display();

int peertcpSocket = -1;	// peer socket

int main(int argc, char **argv) {
	
    int tcpServ_sock;
	
	struct sockaddr_in tcpServer_addr;
	struct sockaddr_in tcpClient_addr;
	struct sockaddr_in newTcp_addr;

	int clnt_len;
		
	fd_set reads, temps;
	int fd_max;

	char command[1024];


	char *tcpport = NULL;
	char *userid = NULL;

	// TODO: NEED TO ADD SOME VARIABLES
	struct hostent *hostp;
	char* input;
	int serv_port, bytesread;
	char* sendmsg = malloc(1024);
	int option;

	if(argc != 3){
		printf("Usage : %s <tcpport> <userid>", argv[0]);
		exit(1);
	}

	tcpport = argv[1];
	userid = argv[2];
	userid[strlen(argv[2])] = '\0';

	display();

	serv_port = atoi(tcpport);
	// TODO: NEED TO CREATE A SOCKET FOR TCP SERVER

	if((tcpServ_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror("tcp server socket\n");
		exit(1);
	}
	option = 1;
	setsockopt(tcpServ_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof option);
	memset((void *) &tcpServer_addr, 0, sizeof tcpServer_addr);
	tcpServer_addr.sin_family = AF_INET;
	tcpServer_addr.sin_addr.s_addr = INADDR_ANY;
	tcpServer_addr.sin_port = htons((u_short)serv_port);
	// TODO: NEED TO bind
	if (bind(tcpServ_sock, (struct sockaddr *)&tcpServer_addr, sizeof tcpServer_addr) < 0)
	{
		perror("bind");
		exit(1);
	}
	// TODO: NEED TO listen
	if (listen(tcpServ_sock, SOMAXCONN) < 0)
	{
		perror("listen");
		exit(1);
	}

	// initialize the select mask variables and set
	// the mask with stdin and the tcp server socket


	FD_ZERO(&reads);	// set all fd to 0
	FD_SET(fileno(stdin), &reads);
	FD_SET(tcpServ_sock, &reads);

	memset(command, 0, sizeof command);
	printf("%s> \n", userid);
	fd_max = tcpServ_sock;

	while(1) {
		int nfound;

		temps = reads;
		
		nfound = select(fd_max+1, &temps, 0, 0, NULL);
		if(nfound == -1)
		{
			perror("select");
			continue;
		}
		if(FD_ISSET(fileno(stdin), &temps)) {
			// Input from the keyboard
			fgets(command, sizeof (command), stdin);
			FD_CLR(fileno(stdin), &temps);
			// NEED TO IMPLEMENT for input from keybord
			
			if(command[0] == '@')
			{
				input = strtok(command + 1, " \n");
				if(strcasecmp(input, "talk") == 0)
				{
					input = strtok(NULL, " \n");
					if((hostp = gethostbyname(input)) == 0) {
						fprintf(stderr,"%s: unknown host\n",input);
						printf("%s> \n", userid);
						continue;
					}
					if((peertcpSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
						perror("peer socket");
						printf("%s> \n", userid);
						continue;
					}
					memset((void *) &tcpClient_addr, 0, sizeof tcpClient_addr);
					tcpClient_addr.sin_family = AF_INET;
					memcpy((void *) &tcpClient_addr.sin_addr, hostp->h_addr, hostp->h_length);
					input = strtok(NULL, " ");
					tcpClient_addr.sin_port = htons((u_short)atoi(input));
					if(connect(peertcpSocket, (struct sockaddr *)&tcpClient_addr, sizeof tcpClient_addr) < 0) {
						FD_CLR(peertcpSocket, &reads);
						(void) close(peertcpSocket);
						fd_max = tcpServ_sock;
						perror("connect");
						continue;
					}
					FD_SET(peertcpSocket, &reads);
					fd_max = peertcpSocket;
				} else if(strcasecmp(input, "quit") == 0) {
					if(peertcpSocket != -1){
						FD_CLR(peertcpSocket, &reads);
						close(peertcpSocket);
						fd_max = tcpServ_sock;
					}
					close(tcpServ_sock);
					return 0;
				} else {
					printf("Cannot find command: %s\n", input);
				}
			} else {
				if(peertcpSocket == -1)
				{
					printf("Connect first\n");
					printf("%s> \n", userid);
					continue;
				}
				strcpy(sendmsg, userid);
				strcat(sendmsg, " : ");
				strcat(sendmsg, command);
				if(write(peertcpSocket, sendmsg, strlen(sendmsg)) < 0)
				{
					perror("write");
					printf("%s> \n", userid);
					continue;
				}
			}
			printf("%s> \n", userid);
			memset(command, 0, sizeof command);
		}
		else if(FD_ISSET(tcpServ_sock, &temps))
		{
			// connect request from a peer -> accept(TODO)
			FD_CLR(tcpServ_sock, &temps);
			clnt_len = sizeof(newTcp_addr);
			peertcpSocket = accept(tcpServ_sock, (struct sockaddr *)&newTcp_addr, &clnt_len);
			if (peertcpSocket < 0) {
				perror("accept");
				continue;
			}
			printf("connection from host %s, port %d, socket %d\n",
				inet_ntoa(newTcp_addr.sin_addr), ntohs(newTcp_addr.sin_port),peertcpSocket);
			FD_SET(peertcpSocket, &reads);
			if (peertcpSocket > fd_max)
				fd_max = peertcpSocket;
			FD_CLR(tcpServ_sock, &temps);
		}
		else if(FD_ISSET(peertcpSocket, &temps))
		{
			// message from a peer
			bytesread = recv(peertcpSocket, command, sizeof command, 0);// read(peertcpSocket, command, sizeof command - 1);
			
			if(bytesread <= 0) {
				printf("Connection Closed %d\n", peertcpSocket);
				FD_CLR(peertcpSocket, &reads);
				if(close(peertcpSocket)) perror("close");
				fd_max = tcpServ_sock;
				continue;
			}
			command[bytesread] = '\0';
			write(fileno(stdout), command, bytesread);
			printf("\n");
			memset(command, 0, sizeof command);
			
			//printf("%s", command);
		}
	}//while End
	close(tcpServ_sock);
	free(sendmsg);
	return 0;
}//main End

void display() {
	printf("Student ID : 20163162 \n");
	printf("Name : Yoonsung Cha	\n");
}