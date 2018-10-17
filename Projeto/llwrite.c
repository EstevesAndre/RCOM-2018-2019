#include "protocol.h"
#include "llwrite.h"

int llwrite(int fd, char* package, int flag)
{
    char BCC2 = calculateBCC2(package, 4 + package[2]*256 + package[3]);

    int char_count;
    char * stuff = stuffing(package, BCC2, &char_count);

    char* message = heading(stuff, char_count, flag);

    send_message(fd, message);

    return 0;
}
