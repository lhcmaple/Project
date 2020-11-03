#ifndef _H_UDP
#define _H_UDP

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>

using namespace std;

class server{
private:
    int sockfd;
    char *buff;
    size_t buffsize;
public:
    server(sockaddr *addr,socklen_t len);
    ~server();
    int run();
};

#endif
