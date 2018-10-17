#include "protocol.h"
#include "llopen.h"
#include "llread.h"

#define BAUDRATE B38400
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


off_t parseMessageStart(char* message, char** filename)
{
    off_t fileSize = 0;
    
    int file_size_length = (int)message[2];

    int i = 1;
    for(; i <= file_size_length; i++)
    {
        fileSize += message[2+i] * pow(256,file_size_length - i);
    }
    
    int filename_length = (int)message[2+i+2];

    char* name = (char *)malloc((filename_length + 1)* sizeof(char));

    i = i + 5;
    int j = 0;
    for(; i <= filename_length; i++, j++)
    {
        name[j] = message[i];
    }

    name[j] = '\0';

    (*filename) = name;

    return fileSize;
}

int parseMessageData(char* message, int messageSize, char** data)
{
    int length = message[2] * 256 + message[3];

    char * dataAux = malloc(length * sizeof(char));

    int i = 4;
    for(; i < length; i++)
    {
        dataAux[i - 4] = message[i];
    }

    (*data) = dataAux;

    return length;
}

void saveData(char* fileContent, char* data, int sizeData, int *index)
{
    int i = 0;
    for(; i < sizeData; i++)
    {
        fileContent[(*index) + i] = data[i];
    }
    
    (*index) += sizeData;    
}

void createFile(char* fileContent, char* filename, off_t size_file)
{
    FILE *file = fopen(filename, "wb+");
    fwrite(fileContent, 1, size_file, file);
    fclose(file);
}


int main(int argc, char** argv)
{
    int fd = setup();

    if(llopen(fd, RECEIVER) == 0)
    {
      printf("Connected\n");
    }
    else
    {
      printf("Failed\n");
    }

    // Initial flag for start package
    int flag = 0;
    char* message;
    int messageSize;

    while((messageSize = llread(fd, flag, &message)) < 0);
    flag = 1;

    char* filename;
    off_t size_file = parseMessageStart(message,&filename);
    
    char * fileContent = malloc(size_file * sizeof(char));

    int index = 0;

    while(index < size_file)
    {
        while((messageSize = llread(fd, flag, &message)) < 0);
        (flag == 0) ? (flag = 1) : (flag = 0);

        char * data;
        int sizeData = parseMessageData(message, messageSize, &data);

        saveData(fileContent, data, sizeData, &index);
    }

    createFile(fileContent, filename, size_file);

    return 0;
}
