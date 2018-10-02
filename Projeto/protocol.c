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

#define BEGIN 0
#define START_MESSAGE 1
#define MESSAGE 2
#define END 3

int flag = 1;

void attend()
{
    flag = 1;
}

int llopen_Receiver(int fd)
{ 
    char BCC1 = A_SENDER ^ C_UA;
    char ua[5] = {FLAG, A_SENDER, C_UA, BCC1, FLAG};

    // ler SET //

    write_message(fd, ua);

    return 0;
}

int llopen_Sender(int fd)
{
    (void) signal(SIGALRM, attend);

    char BCC1 = A_SENDER ^ C_SET;
    char set[5] = {FLAG, A_SENDER, C_SET, BCC1, FLAG};

    int cnt = 0;
    char buf[255];

    while(cnt < 3)
    {
        alarm(3);
        flag = 0;
        
        write_message(fd, buf);
        // ler UA //

        cnt++;
    }


    

}

int read_message(int fd, char buf[], int f)
{
    int state = BEGIN;

    while(state != END)
    {
        if(FLAG != f) return -3;

        switch(state)
        {
            case BEGIN: 
            {
                
                break;
            }
            case START_MESSAGE:
            {
                break;
            }            
            case MESSAGE:
            {
                break;
            }
            default: state = END;
        }
    }

    return 0;
}

int llopen(int fd, int flag)
{    
    if(flag == SENDER)
    {
        return llopen_Sender(fd);
    }
    else if (flag == RECEIVER)
    {
        return llopen_Receiver(fd);
    }
  
    return -1;
}

int write_message(int fd, char buf[])
{    
    if(gets(buf) == NULL)
    {
    	return -2;
    }

    int res = write(fd,buf,255);   
    fflush(NULL);

    sleep(1);

    return 0;
}