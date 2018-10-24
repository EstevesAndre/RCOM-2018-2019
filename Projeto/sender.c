#include "protocol.h"
#include "llopen.h"
#include "llwrite.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

unsigned char n_seq = 0;

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

unsigned char *readFile(unsigned char* filename, off_t *sizeFile)
{
    FILE *file;

    struct stat fileInfo;
    unsigned char* fileContent;

    if( (file = fopen((char*)filename, "rb")) == NULL)
    {
        perror("Error reading file.\n");
        exit(-1);
    }

    stat((char*)filename, &fileInfo);
    (*sizeFile) = fileInfo.st_size;

    fileContent = (unsigned char *)malloc(fileInfo.st_size);

    fread(fileContent, sizeof(unsigned char), fileInfo.st_size, file);

    return fileContent;
}

unsigned char* controlPackage(unsigned char c2, const unsigned char* filename, const off_t sizeFile)
{
    int res = sizeFile / 256;
    int quo = sizeFile % 256;
    int count = 1;

    while(res > 0)
    {
      res = quo / 256;
      quo %= 256;
      count++;
    }

    int size = (5 + strlen((char*)filename) + count) * sizeof(unsigned char);
    unsigned char* data = (unsigned char *)malloc(size);

    data[0] = c2;
    data[1] = T_SIZE;
    data[2] = count;

    res = sizeFile / 256;
    quo = sizeFile % 256;
    int i = 3;
    if(res == 0) data[i] = quo;
    else data[i] = res;

    while(res > 0)
    {
      res = quo / 256;
      quo %= 256;
      i++;
      if(res == 0) data[i] = quo;
      else data[i] = res;
    }

    data[i+1] = T_NAME;
    data[i+2] = strlen((char*)filename);

    i+=3;
    for(count = 0; count < strlen((char*)filename); i++, count++)
    {
        data[i] = filename[count];
    }

    return data;

}

unsigned char* dataPackage(unsigned char * content, off_t *offset, off_t end_offset)
{
    unsigned char* package = malloc(264 * sizeof(unsigned char));

    package[0] = C2_DATA;

    package[1] = n_seq % 255;
    n_seq++;

    off_t chars_to_send = end_offset - *offset;

    if (end_offset - *offset > 260)
    {
        chars_to_send = 260;
    }

    if(chars_to_send > 255)
    {
        package[2] = 1;
        package[3] = chars_to_send - 256;
    }
    else
    {
        package[2] = 0;
        package[3] = chars_to_send;
    }

    int i = 0;
    for(; i < chars_to_send; i++, (*offset)++)
    {
        package[4+i] = content[*offset];
    }

    return package;
}


int main(int argc, char** argv)
{
    if(argc != 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return -1;
    }

    int fd = setup();

    if(llopen(fd, SENDER) == 0)
        printf("Connected\n");
    else
    {
        printf("Failed\n");
        return -2;
    }

    //Opens the file to be sent
    off_t fileSize;
    unsigned char* fileContent;
    fileContent = readFile((unsigned char *)argv[1],&fileSize);

    unsigned char* start = controlPackage(C2_START, (unsigned char *)argv[1], fileSize);
    off_t offsetFile = 0;

    if(llwrite(fd, start, 0,-1) == 2) //ERROR '-1' start package
        return -1;

    int flag = 1;
    int counter = 1;

    while(offsetFile != fileSize)
    {
        unsigned char* package = dataPackage(fileContent, &offsetFile, fileSize);

        flag = llwrite(fd, package,flag, counter);
        counter++;

        if(flag == 2)//ERROR
            return -1;
    }

    printf("Finished to send file %s\n", argv[1]);

    return llclose(fd,SENDER);
}
