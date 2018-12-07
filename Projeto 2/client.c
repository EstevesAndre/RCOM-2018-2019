#include <stdio.h>
#include <stdlib.h>
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include "info.h"

int parseInfo(char* cmd, struct Info* info)
{
    int state = BEGIN;
    int i = 0;
    int aux = 0;
    char c;

    while(state != END)
    {
        c = cmd[i++];
        switch(state)
        {
            case BEGIN:
            {
                if(c == '\0')
                    return -1;

                if(c != 'f')
                    return -2;
                c = cmd[i++];
                if(c != 't')
                    return -2;
                c = cmd[i++];
                if(c != 'p')
                    return -2;
                c = cmd[i++];
                if(c != ':')
                    return -2;
                c = cmd[i++];
                if(c != '/')
                    return -2;
                c = cmd[i++];
                if(c != '/')
                    return -2;
                
                if(strrchr(cmd, '@') != NULL)
                    state = USER;             
                else
                    state = HOST;
                break;
            }
            case USER:
            {
                if(c == '\0')
                    return -1;

                if(c == ':')
                {
                    (*info).user[aux] = 0;
                    aux = 0;
                    state = PASSWORD;
                }
                else
                {
                    (*info).user[aux++] = c;
                }
                break;
            }
            case PASSWORD:
            {
                if(c == '\0')
                    return -1;

                if(c == '@')
                {
                    (*info).password[aux] = 0;
                    aux = 0;
                    state = HOST;
                }
                else
                {
                    (*info).password[aux++] = c;
                }
                break;
            }
            case HOST:
            {
                if(c == '\0')
                    return -1;

                if(c == '/')
                {
                    (*info).hostname[aux] = 0;
                    aux = 0;
                    state = PATH;
                }
                else
                {
                    (*info).hostname[aux++] = c;
                }
                break;
            }
            case PATH:
            {
                if(c == '\0')
                {
                    (*info).path[aux] = 0;
                    aux = 0;
                    state = END;
                }
                else
                {
                    (*info).path[aux++] = c;
                }
                break;
            }
            case END:
            {
                return 0;
                break;
            }
            default: return -1;
        }
    }
    return 0;
}

void getUserInfo(struct Info* info)
{
    char* buf = malloc(50 * sizeof(char));
    size_t size = 50;
    fflush(stdin);
    printf("\nUsername: ");
    getline(&buf, &size, stdin);
    strncpy(info->user, buf, strcspn(buf, "\n"));

    printf("Password: ");
    getline(&buf, &size, stdin);
    strncpy(info->password, buf, strcspn(buf, "\n"));
    printf("\n");
}

int parseFilename(struct Info* info)
{
    char* filename = strrchr(info->path, '/');

    if(filename == NULL)
    {
        info->filename = info->path;
    }
    else
    {
        info->filename = (filename + 1);
    }

    return 0;
}

int getHostInfo(char* hostname, struct hostent** h)
{
    if ((*h=gethostbyname(hostname)) == NULL) 
        return -1;
    
    return 0;
}

int connectTCP(char* addr, int port)
{
    int	sockfd;
	struct sockaddr_in server_addr;
	
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(addr);
	server_addr.sin_port = htons(port);	
    
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) 
        return -1;
        
    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        return -2;
    
    return sockfd;
}

int readResponseCode(int socketfd, char *responseCode)
{
	int state = BEGIN;
	int index = 0;
	char c;

	while (state != END)
	{	
		read(socketfd, &c, 1);
		switch(state)
		{
            case BEGIN:
            {
                if(c == ' ')
                {
                    if (index != 3)
                    {
                        return -1;
                    }
                    index = 0;
                    state = CLEAR_LINE;
                }
                else if(c == '-')
                {
                    state = MULTIPLE_LINE;
                    index = 0;
                }
                else if(isdigit(c))
                {
                    responseCode[index++] = c;
                }
                break;
            }
            case CLEAR_LINE:
            {
                if (c == '\n')
                {
                    state = END;
                }
                break;
            }
            case MULTIPLE_LINE:
            {
                if(c == responseCode[index])
                {
                    index++;
                }
                else if(index == 3 && c == ' ')
                {
                    state = CLEAR_LINE;
                }
                else if(index ==3  && c == '-')
                {
                    index = 0; 
                }
                break;
            }
            case END:
            {
                return 0;
                break;
            }
            default: return -1;
        }
    }
    return 0;
}

void writeCmd(int fd, char* cmd, char* info)
{
    write(fd, cmd, strlen(cmd));
	write(fd, info, strlen(info));
	write(fd, "\n", 1);
}

int writeCommand(int fd, char* cmd, char* info)
{
    char code[3];
    writeCmd(fd, cmd, info);
    readResponseCode(fd, code);
    int answer = code[0] - '0';

    while(1)
    {
        switch(answer)
        {
            case 1:
            {
                if(strcmp(cmd, "retr ")==0)
                    return 2;
                readResponseCode(fd, code);
                break;
            }
            case 2:
                return 0;
            case 3:
                return 1;
            case 4:
            {
                writeCmd(fd, cmd, info);
                break;
            }
            case 5:
                return -1;
        }
    }
}

int sendLoginInfo(struct Info* info, int sockfd)
{
    if(writeCommand(sockfd, CMD_USER, info->user) != 1)
        return -1;
    
    if(writeCommand(sockfd, CMD_PASS, info->password) != 0)
        return -1;

    return 0;
}

int getServerPort(int sockfd)
{
    writeCmd(sockfd, CMD_PASSIVE, "");

    int state = 0;
	int index = 0;
	char msg1[4];
	memset(msg1, 0, 4);
	char msg2[4];
	memset(msg2, 0, 4);

	char c;

	while (state != 7)
	{
		read(sockfd, &c, 1);
		switch (state)
		{
		case 0:
        {
			if (c == ' ')
			{
				if (index != 3)
					return -1;
				index = 0;
				state = 1;
			}
			else
			{
				index++;
			}
			break;
        }
		case 5:
        {
			if (c == ',')
			{
				index = 0;
				state++;
			}
			else
			{
				msg1[index] = c;
				index++;
			}
			break;
        }
		case 6:
        {
			if (c == ')')
			{
				state++;
			}
			else
			{
				msg2[index] = c;
				index++;
			}
			break;
        }
		default:
        {
			if (c == ',')
			{
				state++;
			}
			break;
        }
		}
	}

	int b1 = atoi(msg1);
	int b2 = atoi(msg2);
	return (b1 * 256 + b2);
}

int retrieveFile(struct Info* info, int sockfd, int serverfd)
{
    if(writeCommand(sockfd, CMD_RETRIEVE, info->path) != 2)
        return -1;
    
    FILE *file = fopen(info->filename, "wb+");

	char buffer[1024];
 	int bytes;

    bytes = read(serverfd, buffer, 1024);

 	while (bytes > 0) {
    	bytes = fwrite(buffer, bytes, 1, file);
        bytes = read(serverfd, buffer, 1024);
    }

  	fclose(file);

    return 0;
}

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        printf("Usage: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        return -1;
    }

    printf("%s > Parsing argument...\n", argv[0]);

    struct Info info;

    if(parseInfo(argv[1], &info) != 0)
    {
        printf("Error parsing client info: %s\n", argv[1]);
        return -2;
    }
    
    parseFilename(&info);

    if(info.user[0] == 0)
        getUserInfo(&info);

    printf("%s > Argument parsed successfully\n\n", argv[0]);
    printf("Username: %s\n", info.user);
    printf("Password: %s\n", info.password);     
    printf("Path: %s\n", info.path);
    printf("Filename: %s\n", info.filename);
    printf("Hostname: %s\n\n", info.hostname);

    printf("%s > Fetching host info...\n", argv[0]);

    struct hostent *host;

    if(getHostInfo(info.hostname, &host) != 0)
    {
        printf("Error getting host IP address: %s\n", info.hostname);
        return -3;
    }

    char* addr_str = inet_ntoa(*((struct in_addr *)host->h_addr));

    printf("%s > Host info fetched successfully\n\n", argv[0]);
    printf("Full Hostname: %s\n", host->h_name);
    printf("Host IP: %s\n\n", addr_str); 

    printf("%s > Establishing connection to host...\n", argv[0]);

    int sockfd = connectTCP(addr_str, PORT);

    if(sockfd < 0)
    {
        printf("Error connecting to IP %s using port %d\n", addr_str, PORT);
        return -4;
    }

	char code[3];

    if(readResponseCode(sockfd, code) != 0)
    {
        printf("Error reading server response code\n");
        return -5;
    }

    if(code[0] != '2')
    {
        printf("Error: server responded with code %s\n", code);
        return -6;
    }

    printf("%s > Connection to host established successfully\n\n", argv[0]);

    printf("%s > Sending login information...\n", argv[0]);

    if(sendLoginInfo(&info, sockfd) != 0)
    {
        printf("Error sending login information\n");
        return -7;
    }

    printf("%s > Login information successfully sent\n\n", argv[0]);

    printf("%s > Fetching server port...\n", argv[0]);

    int server_port = getServerPort(sockfd);

    if(server_port < 0)
    {
        printf("Error fetching server port\n");
        return -8;
    }

    printf("%s > Server port fetched successfully\n\n", argv[0]);
    printf("Server port: %d\n\n", server_port);

    printf("%s > Establishing connection to server...\n", argv[0]);

    int serverfd = connectTCP(addr_str, server_port);

    if(serverfd < 0)
    {
        printf("Error connecting to IP %s using port %d\n", addr_str, PORT);
        return -4;
    }

    printf("%s > Connection to server established successfully\n\n", argv[0]);

    printf("%s > Retreiving file...\n", argv[0]);

    if(retrieveFile(&info, sockfd, serverfd) < 0)
    {
        printf("Error retreiving file: %s\n", info.filename);
        return -9;
    }

    printf("%s > File retrieved successfully\n\n", argv[0]);

    printf("%s > Closing...\n", argv[0]);

    close(sockfd);
	close(serverfd);

    return 0;
}