#ifndef _H_UDP
#define _H_UDP

using namespace std;

class client{
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
