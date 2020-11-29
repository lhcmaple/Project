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
#include <pthread.h>
#include <semaphore.h>

using namespace std;

typedef unsigned short ushort;

#define ISNUMBER(c) (c<='9'&&c>='0')
#define TONUMBER(c1,c2) ((c1-'0')*10+(c2-'0'))

#define RWRWRW (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#define RWXRWRW (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#define PATH_LENGTH 256
#define NPOOL 10 //线程池大小
#define BUFFSIZE 65536
#define USERNUM 65536

class server;

struct pthread_arg{
    server *s;
    int id;
}args[NPOOL];

class server{
private:
    static const char *UDPDAT;

    int sockfd;
    char (*buff)[BUFFSIZE];
    size_t nbytes[NPOOL];
    sockaddr_in cliaddr[NPOOL];
    socklen_t len[NPOOL];

    int logfd;
    pthread_mutex_t logmutex=PTHREAD_MUTEX_INITIALIZER;

    char (*userinfo)[3];//需清零
    pthread_mutex_t *usermutex;
    ushort username[NPOOL],password[NPOOL],targetname[NPOOL];

    pthread_t pool[NPOOL];
    sem_t *servsem,*clisem;//<主线程准备好资源,辅线程完成任务>
protected:
    void load();
    int gettype(int);
    void worker(int);
public:
    server(sockaddr *addr,socklen_t len);
    ~server();
    int run();

    friend void *pthread_func(void *);
};

void *pthread_func(void *);

#endif//_H_UDP
