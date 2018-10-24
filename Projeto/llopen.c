#include "protocol.h"
#include "llopen.h"

int llopen_Receiver(int fd)
{
    unsigned char BCC1 = A_SENDER ^ C_UA;
    unsigned char ua[6] = {FLAG, A_SENDER, C_UA, BCC1, FLAG, '\0'};

    disableAlarm();

    unsigned char buf[255];

    while(1)
    {
        if(read_message(fd, buf) == 0) break;
    }

    // analisar sender info
    if(parseMessageType(buf) == C_SET)
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

    unsigned char BCC1 = A_SENDER ^ C_SET;
    unsigned char set[6] = {FLAG, A_SENDER, C_SET, BCC1, FLAG, '\0'};

    int cnt = 0;
    unsigned char buf[255];

    while(cnt < 3)
    {
        alarm(3);
        disableAlarm();

        write_message(fd, set, 5);

        if(read_message(fd, buf) == 0) break;

        cnt++;
    }

    if(cnt == 3)
        return 2; //no confirmation recieved

    // analisar receiver info
    if(parseMessageType(buf) == C_UA)
        return 0;
    else
        return -6;
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
