#include "protocol.h"
#include "llwrite.h"

int llwrite(int fd, char* package, int flag)
{
    char BCC2 = calculateBCC2(package, 4 + package[2]*256 + package[3]);

    int char_count;
    char * stuff = stuffing(package, BCC2, &char_count);

    char* message = heading(stuff, char_count, flag);

    int cnt = 0;
    char buf[255];

    while(cnt < 3)
    {
        alarm(3);
        disableAlarm();

        write_message(fd, message, 5 + char_count);

        if(read_message(fd, buf) == 0)
        {
          if((parseMessageType(buf) == C_RR0 && flag == 1) || (parseMessageType(buf) == C_RR1 && flag == 0))
            break;
        }

        cnt++;
    }

    if(cnt == 3)
    {
        return 2; //no confirmation recieved
    }

    if(flag == 1)
      return 0;
    else
      return 1;
}
