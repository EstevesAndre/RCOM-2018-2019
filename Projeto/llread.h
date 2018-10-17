
int llread(int fd, int flag, unsigned char** message);

int checkBCC2(unsigned char * package, int size);

unsigned char* destuffing(unsigned char* buf, int *size);
