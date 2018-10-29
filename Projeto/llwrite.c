#include "protocol.h"
#include "llwrite.h"

int llwrite(int fd, unsigned char* package, int flag, int noPackage)
{
    unsigned char BCC2;
    if(package[0] == C2_DATA)
        BCC2 = calculateBCC2(package, 4 + package[2]*256 + package[3]);
    else
        BCC2 = calculateBCC2(package, 5 + package[2] + package[2+package[2] + 2]);

    int char_count;
    unsigned char * stuff = stuffing(package, BCC2, &char_count);

    unsigned char* message = heading(stuff, char_count, flag);

    int cnt = 0;
    unsigned char buf[255];

    while(cnt < 3)
    {
        alarm(3);
        disableAlarm();

        write_message(fd, message, 6 + char_count);

        if(read_message(fd, buf) == 0)
        {
            if((parseMessageType(buf) == C_RR0 && flag == 1) || (parseMessageType(buf) == C_RR1 && flag == 0))
            {
                noPackage != -1 ?   printf("Success on sending package no.%d\n", noPackage) :
                                    printf("Success on sending Start package\n");
                break;
            }
        }

        noPackage != -1 ?   printf("Failure on sending package no.%d, try no.%d\n", noPackage,cnt + 1) :
                            printf("Failure on sending Start package, try no.%d\n", cnt + 1);

        cnt++;
    }


    if(cnt == 3)
        return 2; //no confirmation recieved

    if(flag == 1)
        return 0;
    else
        return 1;
}
