#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define FLAG 0x7E
#define A_SENDER 0x03
#define C_SET 0x03
#define C_DISC 0x0B
#define C_UA 0x07

#define SENDER 0
#define RECEIVER 1

int flag = 1;

void attend()
{
    flag = 1;
}

int llopen_Receiver(int fd)
{

}

int llopen_Sender(int fd)
{

}

int llopen(int fd, int flag)
{
    (void) signal(SIGALRM, attend);

    char BCC1 = A_SENDER ^ C_SET;
    char set[5] = {FLAG, A_SENDER, C_SET, BCC1, FLAG};

    if(flag == SENDER)
    {
        llopen_Sender(fd);
    }
    else if (flag == RECEIVER)
    {
        llopen_Receiver(fd);
    }
    else
    {
        return -1;
    }

    

}

int write_message(int fd, char buf[])
{    
    if(gets(buf) == NULL)
    {
    	exit(-1);
    }

    int res = write(fd,buf,255);   
    fflush(NULL);

    printf("%d bytes written\n", res);

    sleep(1);
}