#include "protocol.h"

int alarm_flag = 1;

void attend()
{
    alarm_flag = 1;
}

int llopen_Receiver(int fd)
{ 
    char BCC1 = A_SENDER ^ C_UA;
    char ua[6] = {FLAG, A_SENDER, C_UA, BCC1, FLAG, '\0'};

    alarm_flag = 0;

    char buf[255];
    
    while(1)
    {
        if(read_message(fd, buf) == 0) break;
    }

    // analisar sender info
    if(parseMessage(buf) == C_SET)
    {
        write_message(fd, ua);
        return 0;
    }
    else 
        return -5;
}

int llopen_Sender(int fd)
{
    (void) signal(SIGALRM, attend);

    char BCC1 = A_SENDER ^ C_SET;
    char set[6] = {FLAG, A_SENDER, C_SET, BCC1, FLAG, '\0'};

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
    {        
        return 2; //no confirmation recieved
    }

    // analisar receiver info
    if(parseMessage(buf) == C_UA)
    {        
        return 0;
    }
    else
    {          
        return -6;
    }
}

int read_message(int fd, char buf[])
{
    int state = BEGIN;
    int pos = 0;

    int res;
    char c;

    while(alarm_flag != 1 && state != END)
    {
        res = read(fd,&c,1);
        
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
				buf[pos] = c;
                pos++;
                if(c == FLAG)
                {                    
                    state = END;
                }
                break;
            }
            default: state = END;
        }      
    }

    if(alarm_flag == 1)
        return 1;

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

    write(fd, buf, strlen(buf));
    
    sleep(1);
    
    return 0;
}

char parseMessage(char buf[])
{
    if(buf[0] != FLAG)
        return ERROR;

    if(buf[1] != A_SENDER && buf[1] != A_RECEIVER)
        return ERROR;
    
    if((buf[2] ^ buf[1]) != buf[3])
        return ERROR;

    if(buf[2] == C_DISC || buf[2] == C_SET || buf[2] == C_UA)
    {
        if(buf[4] == FLAG)
            return buf[2];
        else
            return ERROR;
    }

    return ERROR;    
}

unsigned char calculateBBC2(unsigned char *message, int size)
{
    unsigned char bcc2 = message[0];
    int i;
    for(i = 1; i < size; i++)
    {
        bcc2 ^= message[i];
    }
    return bcc2;
}
