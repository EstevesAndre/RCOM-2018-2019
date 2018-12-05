#define BEGIN 0
#define USER 1
#define PASSWORD 2
#define HOST 3
#define PATH 4
#define END 5

struct Info {
    char hostname[100];
    char path[150];
    char user[50];
    char password[50];
};