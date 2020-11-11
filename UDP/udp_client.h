#ifndef _H_UDP
#define _H_UDP

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <dirent.h>
#include <wait.h>
#include <fcntl.h>

using namespace std;

class client{
private:
    int sockfd;
    char *buff;
    size_t buffsize;
    int smesgcount;
    int rmesgcount;

    sockaddr_in servaddr;
    socklen_t len;
public:
    client(const char *servip);
    int run();
    ~client();
};

#endif
