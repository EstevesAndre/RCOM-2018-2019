#include "protocol.h"
#include "llread.h"

int llread(int fd, int flag, unsigned char** message)
{
    unsigned char* buf = malloc(600 * sizeof(unsigned char));

    while(1)
    {
        if(read_message(fd, buf) == 0) break;
    }


    if(buf[2] != (unsigned char)(flag * 64))
    {
        unsigned char c1;
        if(flag == 0) c1 = C_RR0;
        else c1 = C_RR1;

        unsigned char BCC1 = A_SENDER ^ c1;
        unsigned char rr[6] = {FLAG, A_SENDER, c1, BCC1, FLAG, '\0'};

        write_message(fd, rr, 5);

        return -3;
    }

    if(buf[3] != (buf[1] ^ buf[2]))
        return -4;

    int size;

    unsigned char* destuffed = destuffing(buf + 4, &size);

    if (checkBCC2(destuffed, size) == 1)
        return -5;

    *message = destuffed;

    unsigned char c1;
    if(flag == 0) c1 = C_RR1;
    else c1 = C_RR0;

    unsigned char BCC1 = A_SENDER ^ c1;
    unsigned char rr[6] = {FLAG, A_SENDER, c1, BCC1, FLAG, '\0'};

    write_message(fd, rr, 5);
    
    return size;
}

int checkBCC2(unsigned char * package, int size)
{
    int i = 1;
    unsigned char check = package[0];
    
    for(; i < size - 2; i++)
        check ^= package[i];

    if(check == package[size - 2])
        return 0;
    else
        return 1;
}


unsigned char* destuffing(unsigned char* buf, int *size)
{
    unsigned char* destuff = malloc(600);

    int i = 0, j = 0;

    while(1)
    {
        if(buf[i] == 0x7E)
        {
            destuff[j] = buf[i];
            break;
        }
        else if(buf[i] == 0x7D)
        {
            if(buf[i+1] == 0x5E)
                destuff[j] = 0x7E;
            else if(buf[i + 1] == 0x5D)
                destuff[j] = 0x7D;

            j++;
            i+=2;
        }
        else
        {
            destuff[j] = buf[i];
            j++;
            i++;
        }
    }

    (*size) = j + 1;

    return destuff;
}
