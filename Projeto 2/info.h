/**
*   Compilation: gcc -o download client.c
*   Test: ./download ftp://anonymous:1@speedtest.tele2.net/1KB.zip
*/

#define BEGIN 0
#define USER 1
#define PASSWORD 2
#define HOST 3
#define PATH 4
#define END 5

#define PORT 21

struct Info {
    char hostname[100];
    char path[150];
    char* filename;
    char user[50];
    char password[50];
};