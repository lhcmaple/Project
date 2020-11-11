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

typedef unsigned short ushort;

using namespace std;
#define RWRWRW (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#define RWXRWRW (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#define PATH_LENGTH 256

class server{
private:
    static const char *UDPDAT;

    char path[PATH_LENGTH];
    char path1[PATH_LENGTH];

    int sockfd;
    char *buff;
    size_t buffsize;
    sockaddr_in cliaddr;
    socklen_t len;

    unordered_map<ushort,ushort> userinfo;
    ushort username,password,targetname;
protected:
    void load();
    int gettype();
    void worker(int type,size_t nbytes);
public:
    server(sockaddr *addr,socklen_t len);
    ~server();
    int run();
};

#endif
