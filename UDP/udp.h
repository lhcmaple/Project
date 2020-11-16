#ifndef _H_UDP
#define _H_UDP

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <unordered_map>
#include <cstring>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctime>
#include <dirent.h>
#include <sys/shm.h>

using namespace std;

typedef unsigned short ushort;
typedef char (*TYPE_USERINFO)[3];

#define RWRWRW (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#define RWXRWRW (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#define PATH_LENGTH 256
#define NPOOL 10 //进程池大小

class server{
private:
    static const char *UDPDAT;

    char path[PATH_LENGTH];
    char path1[PATH_LENGTH];

    int sockfd;//进程池共享
    char *buff;//需要传送到子进程
    size_t buffsize;//进程池共享
    sockaddr_in cliaddr;//需要传送到子进程
    socklen_t len;//需要传送到子进程

    TYPE_USERINFO userinfo;//共享存储解决同步修改问题
    ushort username,password,targetname;

    pid_t pool[NPOOL];
    pid_t pfd[NPOOL][2];
protected:
    void load();
    int gettype();
    void worker(int rfd);
public:
    server(sockaddr *addr,socklen_t len);
    ~server();
    int run();
};

#endif//_H_UDP
