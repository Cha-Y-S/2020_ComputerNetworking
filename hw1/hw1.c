// Student ID : 20163162
// Name : Yooonsung Cha

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

#define PROMPT() {printf("\n> ");fflush(stdout);}
#define GETCMD "dl"
#define QUITCMD "quit"
#define BUFSIZE 1024

int main(int argc, char *argv[]) {
	int socktoserver = -1;
	char buf[BUFSIZ];

	char fname[BUFSIZ];	

	printf("Student ID : 20163162\n");
	printf("Name : Yoonsung Cha\n");

	PROMPT(); 

	for (;;) {
		if (!fgets(buf, BUFSIZ - 1, stdin)) {
			if (ferror(stdin)) {
				perror("stdin");
				exit(1);
			}
			exit(0);
		}

		char *cmd = strtok(buf, " \t\n\r");

		if((cmd == NULL) || (strcmp(cmd, "") == 0)) {
			PROMPT(); 
			continue;
		} else if(strcasecmp(cmd, QUITCMD) == 0) {
				exit(0);
		}

		if(!strcasecmp(cmd, GETCMD) == 0) {
			printf("Wrong command %s\n", cmd);
			PROMPT(); 
			continue;
		}

		// connect to a server
		char *hostname = strtok(NULL, " \t\n\r");
		char *pnum = strtok(NULL, " ");
		char *filename = strtok(NULL, " \t\n\r");

		// just for Debugging, print the URL
		//printf("%s:%s%s\n", hostname, pnum, filename);

		// NEED TO IMPLEMENT HERE
		struct hostent *hostp;
		struct sockaddr_in server;
		int sock;

		// Create Socket
		if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		{
			perror("socket\n");
			exit(1);
		}

		// Get host name
		if((hostp = gethostbyname(hostname)) == 0)
		{
			fprintf(stderr,"%s: unknown host\n",hostname);
			fprintf(stderr, "cannot connect to server %s\n",hostname);
			PROMPT();
			continue;
		}

		memset((void *) &server, 0, sizeof(server));
		server.sin_family = AF_INET;
		memcpy((void *) &server.sin_addr, hostp->h_addr, hostp->h_length);
		server.sin_port = htons((u_short)atoi(pnum));

		// Connect to server
		if(connect(sock,(struct sockaddr *)&server, sizeof(server)) < 0)
		{
			(void) close(sock);
			fprintf(stderr, "cannot connect to server %s",hostname);
			PROMPT();
			continue;
		}

		// Create HTTP request message
		char *command = "GET ";
		char *http = " HTTP/1.0\r\n";
		char *host = "Host: ";
		char *othermsg = "\r\nUser-agent: HW1/1.0\r\nID: 20163162\r\nName: Yoonsung Cha\r\nConnection: close\r\n\r\n";
		char msg[BUFSIZE];
		char response_msg[BUFSIZE];

		sprintf(msg,"%s%s%s%s%s%s", command, filename, http, host, hostname, othermsg);

		// Send HTTP request message		
		if(write(sock, msg, strlen(msg)) < 0)
		{
			perror("write");
			PROMPT();
			continue;
		}
		// printf("request message\n%s",msg); 

		// Recieve HTTP response message
		int size = recv(sock,response_msg, sizeof response_msg, 0);
		if(size < 0)
		{
			perror("receive");
			PROMPT();
			continue;
		}
		response_msg[size] = '\0';
		/* printf("==========response message==========\n");
		printf("%s\n",response_msg);
		printf("====================================\n"); */

		// Get HTTP Status Code
		int i = 9;

		char *status = malloc(3);

		strncpy(status, response_msg + 9, 3);
		int status_code = atoi(status);
		int start_response = strlen(response_msg);

		switch(status_code)
		{
		case 200:
			break;
		case 301:
			printf("%d Moved Permanently\n", status_code);
			PROMPT();
			continue;
			break;
		case 400:
			printf("%d Bad request\n", status_code);
			PROMPT();
			continue;
			break;
		case 404:
			printf("%d Not Found\n", status_code);
			PROMPT();
			continue;
			break;
		case 505:
			printf("%d HTTP Version Not Supported\n", status_code);
			PROMPT();
			continue;
			break;
		}
		free(status);
		
		// Get File name
		strcpy(fname,filename);
		char *file = strtok(fname, "/");
		fname[0] = '\0';
		
		while(file!=NULL)
		{
			strcpy(fname,file);
			file = strtok(NULL, "/");
		}

		// Get Content Length
		char *filesize = strtok(response_msg, "\n");

		while(filesize!=NULL)
		{
			filesize = strtok(NULL, ":");
			if(strcmp(filesize, "Content-Length") == 0)
			{
				filesize = strtok(NULL,"\n");
				break;
			}
			filesize = strtok(NULL, "\n");
		}


		int content_length = atoi(filesize);
		printf("Total Size %d bytes\n",content_length);


		char *receive_data = strtok(NULL, "\n");
		// printf("receive_data: %s\n", receive_data);
		while(receive_data[0] != '\r' && receive_data[1] != 0)
		{
			receive_data = strtok(NULL, "\n");
		}
		int dl_file_size = 0;

				
		FILE *fp = fopen(fname, "wb");

		// Get data from HTTP response header
		while(*receive_data == 0)
		{
			//printf("receive data: %s\n", receive_data);
			receive_data += 1;
		} 
		receive_data += 2;

		//printf("receive data: %s\n", receive_data);

		int start_data = strlen(receive_data);
		// printf("size: %d\n", start_response - start_data);
		
		// Don't write if HTTP response header has no data
		if(*receive_data != 0)
		{
			fwrite(receive_data, size - (start_response - start_data), 1, fp);
		}
			
		i  = 1;
		while(1)
		{
			// Receive data left in socket
			memset(response_msg, 0, sizeof response_msg);
			int recv_data_size = recv(sock, response_msg, sizeof response_msg, 0);
			response_msg[recv_data_size] = '\0';
			
			// Complete download if downloaded file size is same or bigger than content length
			if(dl_file_size >= content_length)
			{
				printf("Download Complete: %s, %d/%d\n", fname, dl_file_size, content_length);
				break;
			}
			
			// Write received data from socket
			fwrite(response_msg, recv_data_size, 1, fp);

			// Check current download size
			dl_file_size = ftell(fp);

			if(dl_file_size >= (content_length / 10) * i)
			{
				printf("Current Downloading %d/%d (bytes) %d%%\n", dl_file_size, content_length, (int) (dl_file_size * 100 / content_length));
				i++;
			}
		}
		
		// close file pointer, socket
		fclose(fp);
		close(sock);
		
		PROMPT();
	}
} 
