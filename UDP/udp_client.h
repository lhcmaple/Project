#ifndef _H_UDP
#define _H_UDP

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>

using namespace std;

class client{
private:
    int sockfd;
    char *buff;
    size_t buffsize;

    sockaddr_in servaddr;
    socklen_t len;
    sockaddr_in cliaddr;
    socklen_t len_client;
public:
    client();
    int run();
    ~client();
};

#endif
