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
                state = USER;             
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

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        printf("Usage: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        return -1;
    }

    struct Info info;

    if(parseInfo(argv[1], &info) != 0)
    {
        printf("Error parsing client info: %s\n", argv[1]);
        return -2;
    }

    parseFilename(&info);

    struct hostent *host;

    if(getHostInfo(info.hostname, &host) != 0)
    {
        printf("Error getting host IP address: %s\n", info.hostname);
        return -3;
    }

    char* addr_str = inet_ntoa(*((struct in_addr *)host->h_addr));

    int sockfd = connectTCP(addr_str, PORT);

    if(sockfd < 0)
    {
        printf("Error connecting to IP %s using port %d\n", addr_str, PORT);
        return -4;
    }

    printf("Username: %s\n", info.user);
    printf("Password: %s\n", info.password);     
    printf("Path: %s\n", info.path);
    printf("Filename: %s\n", info.filename);
    printf("Hostname: %s\n", info.hostname);     
    printf("Full Hostname: %s\n", host->h_name);
    printf("Host IP: %s\n", addr_str);     
    return 0;
}