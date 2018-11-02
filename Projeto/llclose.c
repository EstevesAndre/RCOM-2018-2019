#include "protocol.h"
#include "llclose.h"

int llclose(int fd, int flag)
{
    if(flag == RECEIVER)
        return llclose_Receiver(fd);
    else
        return llclose_Sender(fd);  

    return -1;
}

int llclose_Receiver(int fd)
{
    unsigned char BCC1 = A_RECEIVER ^ C_DISC;
    unsigned char disc[6] = {FLAG, A_RECEIVER, C_DISC, BCC1, FLAG, '\0'};

    unsigned char buf[255];

    while(1)
    {
        if(read_message(fd,buf) == 0) break;
    }

    if(parseMessageType(buf) == C_DISC)
        write_message(fd,disc,5);
    else
        return -5;


    while(1)
    {
        if(read_message(fd,buf) == 0) break;
    }

    if(parseMessageType(buf) == C_UA)
    {
        close(fd);
        return 0;
    }
    
    return -6;
}

int llclose_Sender(int fd)
{
    unsigned char BCC1 = A_SENDER ^ C_DISC;
    unsigned char disc[6] = {FLAG, A_SENDER, C_DISC, BCC1, FLAG, '\0'};

    write_message(fd,disc,5);

    unsigned char buf[255];

    while(1)
    {
        if(read_message(fd,buf) == 0) break;
    }

    BCC1 = A_SENDER ^ C_UA;
    unsigned char ua[6] = {FLAG, A_SENDER, C_UA, BCC1, FLAG, '\0'};

    if(parseMessageType(buf) == C_DISC)
    {
        write_message(fd,ua,5);
        close(fd);
        return 0;
    }
        
    return -5;       
}