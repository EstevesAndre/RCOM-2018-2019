#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <math.h>

#define FLAG 0x7E
#define A_SENDER 0x03
#define A_RECEIVER 0x01
#define C_SET 0x03
#define C_DISC 0x0B
#define C_UA 0x07
#define C_RR0 0x05
#define C_RR1 0x85
#define C_REJ0 0x01
#define C_REJ1 0x81

#define ERROR 0xFF

#define SENDER 0
#define RECEIVER 1

#define BEGIN 0
#define START_MESSAGE 1
#define MESSAGE 2
#define END 3

#define C2_START 0x02
#define C2_DATA 0x01
#define C2_END 0x03
#define T_SIZE 0x00
#define T_NAME 0x01

void attend();

void disableAlarm();

int read_message(int fd, char buf[]);

void write_message(int fd, char buf[], int size);

char parseMessageType(char buf[]);

char calculateBCC2(char *message, int size);

char* stuffing_data_package(const char* package, const char BCC2, int* char_count);

char* stuffing_control_package(const char* package, const char BCC2, int* char_count);

char* stuffing(const char* package, const char BCC2, int* char_count);

char* heading(char * stuff, int count, int flag);

#endif
