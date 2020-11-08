#ifndef _H_UDP
#define _H_UDP

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <unordered_map>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctime>
#include <dirent.h>

typedef unsigned short ushort;

using namespace std;
#define RWRWRW (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#define RWXRWRW (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

class server{
private:
    char path[256];
    char path1[256];
    int sockfd;
    char *buff;
    size_t buffsize;
    unordered_map<ushort,ushort> userinfo;
protected:
    void load();
    int gettype();
public:
    server(sockaddr *addr,socklen_t len);
    ~server();
    int run();
};

#endif
