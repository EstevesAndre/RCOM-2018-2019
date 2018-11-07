#include "protocol.h"
#include "llopen.h"
#include "llread.h"
#include "llclose.h"

#define BAUDRATE B19200
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int setup()
{
    int fd;
    struct termios oldtio,newtio;

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

    fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY );
    fflush(NULL);

    if (fd <0) {perror("/dev/ttyS0"); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) pr�ximo(s) caracter(es)
  */

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

    return fd;
}


off_t parseMessageStart(unsigned char* message, unsigned char** filename)
{
    off_t fileSize = 0;

    int file_size_length = message[2];

    int i = 1;
    for(; i <= file_size_length; i++)
        fileSize += message[2+i] * pow(256,file_size_length - i);

    int filename_length = message[2+i+1];

    unsigned char* name = (unsigned char *)malloc((filename_length + 1)* sizeof(unsigned char));

    int k = i + 4;
    int j = 0;
    for(; k < filename_length + i + 4; k++, j++)
        name[j] = message[k];

    name[j] = '\0';

    (*filename) = name;

    printf("Number of Packages = %li\n", fileSize / 260 + 1);
    printf("Filename = %s\n", *filename);

    return fileSize;
}

int parseMessageData(unsigned char* message, int messageSize, unsigned char** data)
{
    int length = message[2] * 256 + message[3];

    unsigned char * dataAux = malloc(length * sizeof(unsigned char));

    int i = 4;
    for(; i < length + 4; i++)
        dataAux[i-4] = message[i];

    (*data) = dataAux;

    return length;
}

void saveData(unsigned char* fileContent, unsigned char* data, int sizeData, int *index)
{
    int i = 0;
    for(; i < sizeData; i++)
        fileContent[(*index) + i] = data[i];

    (*index) += sizeData;
}

void createFile(unsigned char* fileContent, unsigned char* filename, off_t size_file)
{
    FILE *file = fopen((char*)filename, "wb+");
    fwrite(fileContent, 1, size_file, file);
    fclose(file);
}


int main(int argc, char** argv)
{
    int fd = setup();

    if(llopen(fd, RECEIVER) == 0)
        printf("Connected\n");
    else
    {
        printf("Failed\n");
        return -2;
    }

    // Initial flag for start package
    int flag = 0;
    unsigned char* message;
    int messageSize;

    while((messageSize = llread(fd, flag, &message)) < 0);
    flag = 1;

    unsigned char* filename;
    off_t size_file = parseMessageStart(message,&filename);

    unsigned char * fileContent = malloc(size_file * sizeof(unsigned char));

    int index = 0;
    int counter = 0;

    while(index < size_file)
    {
        while((messageSize = llread(fd, flag, &message)) < 0);
        (flag == 0) ? (flag = 1) : (flag = 0);

        unsigned char * data;
        int sizeData = parseMessageData(message, messageSize, &data);
        counter++;

        saveData(fileContent, data, sizeData, &index);

        printf("Received package no.%d\n",counter);
    }

    createFile(fileContent, filename, size_file);

    printf("Finished receiving file %s\n", filename);

    return llclose(fd,RECEIVER);
}
