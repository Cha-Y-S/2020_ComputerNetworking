// Student ID : 20163162
// Name : Yoonsung Cha

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

int main(int argc, char *argv[]) {
	struct sockaddr_in server, remote;
	int request_sock, new_sock;
	int bytesread;
	socklen_t addrlen;
	char buf[BUFSIZ];

	if (argc != 2) {
		(void) fprintf(stderr,"usage: %s portnum \n",argv[0]);
		exit(1);
	}

	int portnum = atoi(argv[1]);

	if ((request_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket");
		exit(1);
	}
	
	printf("Student ID : 20163162\n");
	printf("Name : Yoonsung Cha\n");

	// Create a Server Socket
	memset((void *) &server, 0, sizeof (server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons((u_short)portnum);

	if (bind(request_sock, (struct sockaddr *)&server, sizeof (server)) < 0) {
		perror("bind");
		exit(1);
	}
	if (listen(request_sock, SOMAXCONN) < 0) {
		perror("listen");
		exit(1);
	}

	while(1) {
		/* a new connection is available on the connetion socket */
		addrlen = sizeof(remote);

		new_sock = accept(request_sock, (struct sockaddr*)&remote, &addrlen);
		if(new_sock < 0)
		{
			perror("accept");
			continue;
		}
		while(1)
		{
			char *cmd;
			char *request_file;
			int status_code;
			char *http_version;
			char *http_response_msg;
			char *request_file_type;
			char buf_copy[BUFSIZ];
			int file_size;	
			
			printf("Connection: Host IP %s, Port %d, socket %d\n", inet_ntoa(remote.sin_addr), ntohs(remote.sin_port), new_sock);
			bytesread = read(new_sock, buf, sizeof (buf) - 1);
			if (bytesread<=0) {
				if (close(new_sock)) 
					perror("close");
				break;
			}
			buf[bytesread] = '\0';
			strcpy(buf_copy, buf);
			cmd = strtok(buf, " \t\n\r");
			request_file = strtok(NULL, " \t\n\r");
			request_file += 1;
			http_version = strtok(NULL, " \t\n\r");

			int i;
			for(i = 0;i<strlen(request_file);i++)
			{
				if(request_file[i] == '.')
				{
					break;
				}
			}
			request_file_type = request_file + i;

			if(!strcasecmp(request_file_type,".html"))
			{
				printf("%s",buf_copy);
			}
			
			// write file to socket
			FILE *fp;
			if((fp = fopen(request_file, "r")) == NULL)
			{
				status_code = 404;
				http_response_msg = "Not Found";
				file_size = 0;
				if(!strcasecmp(request_file_type,".html"))
				{
					printf("Server Error : No such file ./%s\n",request_file);
				}
			}
			else
			{
				if(!strcasecmp(cmd,"GET") && !strcasecmp(cmd, "HEAD"))
				{
					status_code = 400;
					http_response_msg = "Bad request";
					file_size = 0;
					printf("Bad request\n");
				}
				else
				{
					fseek(fp, 0, SEEK_END);
					file_size = ftell(fp);
					rewind(fp);
					status_code = 200;
					http_response_msg = "OK";
				}
			}
			
			char *response_http_version = "HTTP/1.0 ";
			char *connection = "\r\nConnection: close\r\n";
			char *ID = "ID: 20163162\r\n";
			char *name = "Name: Yoonsung Cha\r\n";
			char *content_length = "Content-Length: ";
			char *content_type = "\r\nContent-Type: text/html\r\n\r\n\0";
			char response_msg[BUFSIZ];

			sprintf(response_msg,"%s%d %s%s%s%s%s%d%s"
				, response_http_version
				, status_code
				, http_response_msg
				, connection
				, ID
				, name
				, content_length
				, file_size
				, content_type);

			if(status_code == 200)
			{
				int send_length = 0;
				write(new_sock, response_msg, strlen(response_msg));
				while(send_length != file_size){
					memset(buf, 0, sizeof(buf));
					fread(buf, 1, 1, fp);
					buf[1] = '\0';
					write(new_sock, buf, 1);
					send_length += 1;
				}
				printf("finish %d %d\n", file_size, send_length);
				fclose(fp);
			}
			else
			{
				write(new_sock, response_msg, strlen(response_msg));
			}
			close(new_sock);
			break;
		}
	}
	close(request_sock);

	return 0;
} /* main - hw2.c */