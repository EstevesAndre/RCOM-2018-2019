/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, unsigned char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    unsigned char buf[255];

    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0))) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    fflush(NULL);
    if (fd <0) {perror(argv[1]); exit(-1); }

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

    // int n=0;
    // while (STOP==FALSE) {       /* loop for input */
    //   res = read(fd,buf,1);   /* returns after 5 chars have been input */
    //   if(res==1){
    //     if (buf[0]=='\0') break;                        /* so we can printf... */
    //     printf("%d\n", n);
    //     n++;
    //
    //     printf("%c\n", buf[0]);
    //     }
    // }

    int i = 0;
    while (STOP==FALSE) {       /* loop for input */
      res = read(fd,buf + i,1);   /* returns after 5 chars have been input */
      if(res>0)
      {
        if (buf[i]=='\0') STOP=TRUE;
        i++;
      }
}

    printf("%s\n", buf);



  /*
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o
  */


    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
