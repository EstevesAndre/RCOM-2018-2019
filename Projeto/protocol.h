#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define FLAG 0x7E
#define A_SENDER 0x03
#define A_RECEIVER 0x01
#define C_SET 0x03
#define C_DISC 0x0B
#define C_UA 0x07
#define ERROR 0xFF

#define SENDER 0
#define RECEIVER 1

#define BEGIN 0
#define START_MESSAGE 1
#define MESSAGE 2
#define END 3

void attend();

int llopen_Receiver(int fd);

int llopen_Sender(int fd);

int read_message(int fd, char buf[]);

int llopen(int fd, int flag);

int write_message(int fd, char buf[]);

char parseMessage(char buf[]);
