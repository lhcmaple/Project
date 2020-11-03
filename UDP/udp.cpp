#include "udp.h"

int main(int argc,char *argv[])
{
    struct sockaddr_in servaddr;
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=50000;
    // inet_aton("42.192.55.215",&(servaddr.sin_addr));
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    server s((sockaddr*) &servaddr,sizeof(servaddr));
    return s.run();
}

server::server(sockaddr *addr,socklen_t len)
{
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    bind(sockfd,addr,len);
    buffsize=(1<<16);
    buff=new char[buffsize];
}

int server::run()
{
    cout<<"服务器启动..."<<endl;
    sockaddr_in cliaddr;
    socklen_t len;
    size_t nbytes;
    while(true)
    {
        nbytes=recvfrom(sockfd,buff,buffsize,0,(sockaddr *)&cliaddr,&len);
        cout<<"接收到消息"<<endl;
        sendto(sockfd,buff,nbytes,0,(sockaddr *) &cliaddr,len);
    }
    return 0;
}

server::~server()
{
    delete[] buff;
}
