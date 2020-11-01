#ifndef _H_UDP
#define _H_UDP

#include <sys/socket.h>

class server{
private:
    int sockfd;
    char *buff;
    size_t nbytes;
    int flags;
    sockaddr from;
    socklen_t addrlen;
public:
    server();
};

#endif
