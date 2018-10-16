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
        write_message(fd, ua, 5);
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
        
        write_message(fd, set, 5);

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

int write_message(int fd, char buf[], int size)
{    

    write(fd, buf, size);
    
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

char calculateBCC2(char *message, int size)
{
    char bcc2 = message[0];
    int i = 1;

    for(; i < size; i++)
        bcc2 ^= message[i];

    return bcc2;
}

int stuffing_data_package(const char* package, const char BCC2, char* stuff)
{
    stuff = (char *)malloc(265 * 2 * sizeof(char));
    
    int char_count = 1;

    stuff[0] = package[0];
    
    if(package[1] == 0x7E)
    {
        stuff[char_count++] = 0x7D;
        stuff[char_count++] = 0x5E;
    }
    else if(package[1] == 0x7D)
    {
        stuff[char_count++] = 0x7D;
        stuff[char_count++] = 0x5D;
    }
    else
    {
        stuff[char_count++] = package[1];
    }

    stuff[char_count++] = package[2];

    if(package[3] == 0x7E)
    {
        stuff[char_count++] = 0x7D;
        stuff[char_count++] = 0x5E;
    }
    else if(package[3] == 0x7D)
    {
        stuff[char_count++] = 0x7D;
        stuff[char_count++] = 0x5D;
    }
    else
    {
        stuff[char_count++] = package[3];
    }
    
    int count = 4;
    int i = package[2] * 256 + package[3];

    for(; count < 4 + i; count++)
    {        
        if(package[count] == 0x7E)
        {
            stuff[char_count++] = 0x7D;
            stuff[char_count++] = 0x5E;
        }
        else if(package[count] == 0x7D)
        {
            stuff[char_count++] = 0x7D;
            stuff[char_count++] = 0x5D;
        }
        else
        {
            stuff[char_count++] = package[count];
        }
    }

    if(BCC2 == 0x7E)
    {
        stuff[char_count++] = 0x7D;
        stuff[char_count++] = 0x5E;
    }
    else if(BCC2 == 0x7D)
    {
        stuff[char_count++] = 0x7D;
        stuff[char_count++] = 0x5D;
    }
    else
    {
        stuff[char_count++] = BCC2;
    }
    
    return char_count;
}

int stuffing_control_package(const char* package, const char BCC2, char* stuff)
{
    int size = package[2];

    stuff = (char *)malloc( (5 + size + package[3+size] * 256 + package[4+size]) * 2 * sizeof(char) );
    
    int char_count = 1;

    stuff[0] = package[0];

    if(package[1] == 0x7E)
    {
        stuff[char_count++] = 0x7D;
        stuff[char_count++] = 0x5E;
    }
    else if(package[1] == 0x7D)
    {
        stuff[char_count++] = 0x7D;
        stuff[char_count++] = 0x5D;
    }
    else
    {
        stuff[char_count++] = package[1];
    }

    if(package[2] == 0x7E)
    {
        stuff[char_count++] = 0x7D;
        stuff[char_count++] = 0x5E;
    }
    else if(package[2] == 0x7D)
    {
        stuff[char_count++] = 0x7D;
        stuff[char_count++] = 0x5D;
    }
    else
    {
        stuff[char_count++] = package[2];
    }

    int count = 3;

    for(; count < (3+size); count++)
    {        
        if(package[count] == 0x7E)
        {
            stuff[char_count++] = 0x7D;
            stuff[char_count++] = 0x5E;
        }
        else if(package[count] == 0x7D)
        {
            stuff[char_count++] = 0x7D;
            stuff[char_count++] = 0x5D;
        }
        else
        {
            stuff[char_count++] = package[count];
        }
    }

    if(package[3+size] == 0x7E)
    {
        stuff[char_count++] = 0x7D;
        stuff[char_count++] = 0x5E;
    }
    else if(package[3+size] == 0x7D)
    {
        stuff[char_count++] = 0x7D;
        stuff[char_count++] = 0x5D;
    }
    else
    {
        stuff[char_count++] = package[3+size];
    }

    if(package[4+size] == 0x7E)
    {
        stuff[char_count++] = 0x7D;
        stuff[char_count++] = 0x5E;
    }
    else if(package[4+size] == 0x7D)
    {
        stuff[char_count++] = 0x7D;
        stuff[char_count++] = 0x5D;
    }
    else
    {
        stuff[char_count++] = package[4+size];
    }

    count = 5 + size;
    int start_pnt = count;
    size = package[4+size];

    for(; count < (start_pnt + size); count++)
    {        
        if(package[count] == 0x7E)
        {
            stuff[char_count++] = 0x7D;
            stuff[char_count++] = 0x5E;
        }
        else if(package[count] == 0x7D)
        {
            stuff[char_count++] = 0x7D;
            stuff[char_count++] = 0x5D;
        }
        else
        {
            stuff[char_count++] = package[count];
        }
    }  

    if(BCC2 == 0x7E)
    {
        stuff[char_count++] = 0x7D;
        stuff[char_count++] = 0x5E;
    }
    else if(BCC2 == 0x7D)
    {
        stuff[char_count++] = 0x7D;
        stuff[char_count++] = 0x5D;
    }
    else
    {
        stuff[char_count++] = BCC2;
    }  

    return char_count;
}

int stuffing(const char* package, const char BCC2, char* stuff)
{ 
    if(package[0] == C2_DATA)
    {
        return stuffing_data_package(package, BCC2, stuff);
    }
    else
    {
        return stuffing_control_package(package, BCC2, stuff);
    }
}

char* heading(char * stuff, int count, int flag)
{
    char * message = (char *)malloc( (5 + count) * sizeof(char));

    message[0] = FLAG;
    message[1] = A_SENDER;
    message[2] = (char)(flag * 64);  
    message[3] = A_SENDER ^ message[2];

    int i = 4;
    
    for(; i < 5 + count; i++)
    {
        message[i] = stuff[i - 4];
    }

    message[i] = FLAG;

    return message;
}

int llwrite(int fd, char* package, int flag)
{
    char BCC2 = calculateBCC2(package, 4 + package[2]*256 + package[3]);

    char * stuff = NULL;

    int char_count = stuffing(package, BCC2, stuff);
    
    char* message = heading(stuff, char_count, flag);



    return 0;
}