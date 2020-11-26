#include <sys/unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <memory.h>
#include <stdlib.h>

/**
* @usage:ip_ttl host port ttl
**/
int main(int argc,char *argv[])
{
    if(argc<3)
        return -1;

    int sockfd=socket(AF_INET,SOCK_DGRAM,0);

    int ttl=atoi(argv[3]);
    setsockopt(sockfd,IPPROTO_IP,IP_TTL,&ttl,sizeof(ttl));

    sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(atoi(argv[2]));
    inet_aton(argv[1],&server_addr.sin_addr);
    
    sendto(sockfd,NULL,0,0,(sockaddr *)&server_addr,sizeof(server_addr));
    close(sockfd);
    return 0;
}