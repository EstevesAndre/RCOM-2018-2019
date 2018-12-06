/**
*   Compilation: gcc -o download client.c
*   Test: ./download ftp://anonymous:1@speedtest.tele2.net/1KB.zip
*/

#define BEGIN 0
#define END 99

#define USER 1
#define PASSWORD 2
#define HOST 3
#define PATH 4

#define CLEAR_LINE 1
#define MULTIPLE_LINE 2

#define PORT 21

#define CMD_USER "user "
#define CMD_PASS "pass "
#define CMD_PASSIVE "pasv"

struct Info {
    char hostname[100];
    char path[150];
    char* filename;
    char user[50];
    char password[50];
};