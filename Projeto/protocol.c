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

int alarm_flag = 1;

void attend()
{
    alarm_flag = 1;
}

int llopen_Receiver(int fd)
{ 
    char BCC1 = A_SENDER ^ C_UA;
    char ua[5] = {FLAG, A_SENDER, C_UA, BCC1, FLAG};

    alarm_flag = 0;

    char buf[255];
    
    while(1)
    {
        if(read_message(fd, buf) == 0) break;
    }

    // analisar sender info

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
        alarm_flag = 0;
        
        write_message(fd, set);
        if(read_message(fd, buf) == 0) break;

        cnt++;
    }

    if(cnt == 3)
        return 2; //no confirmation recieved

    // analisar receiver info

    

}

int read_message(int fd, char buf[])
{
    int state = BEGIN;
    int pos = 0;

    int res;
    char c;

    while(state != END)
    {
        res = read(fd,c,1);
        
        if(res > 0)
        {
            switch(state)
            {
                case BEGIN: 
                {
                    if(c == FLAG)
                    {
                        buf[pos] = c;
                        pos++;
                        state = START_MESSAGE;
                    }                    
                    break;
                }
                case START_MESSAGE:
                {                    
                    if(c != FLAG)
                    {
                        buf[pos] = c;
                        pos++;
                        state = MESSAGE;
                    }
                    break;
                }            
                case MESSAGE:
                {
                    if(c == FLAG)
                    {
                        buf[pos] = c;
                        pos++;
                        state = END;
                    }
                    break;
                }
                default: state = END;
            }
        }

        if(alarm_flag == 1)
            return 1;

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