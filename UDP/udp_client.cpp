#include "udp_client.h"

using namespace std;

int main(int argc,char *argv[])
{
    client c(argv[1]);
    return c.run(argv[2]);
}

client::client(const char *servip)
{
    sockfd=socket(AF_INET,SOCK_DGRAM,0);

    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(5000);
    if(!servip)
        servip="127.0.0.1";
    inet_aton(servip,&(servaddr.sin_addr));
    len=sizeof(servaddr);

    buffsize=(1<<16);
    buff=new char[buffsize];
}

int client::run(const char *file)
{
    timeval tv;
    tv.tv_sec=5;
    tv.tv_usec=0;
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));

    size_t nbytes;
    int filefd;
    sprintf(buff,"udp.tsdat/%s",file);
    filefd=open(buff,O_RDONLY);
    nbytes=read(filefd,buff,buffsize);
    sendto(sockfd,buff,nbytes,0,(sockaddr *)&servaddr,len);
    nbytes=recvfrom(sockfd,buff,buffsize-1,0,0,0);
    if(nbytes<=0)
    {
        // if(errno==EAGAIN||errno==EWOULDBLOCK)
        //     printf("进程%d超时\n",getpid());
        exit(-1);//接收不到消息
    }
    else
    {
        buff[nbytes]='\0';
        cout<<buff<<endl;
        exit(0);
    }
    return 0;
}

client::~client()
{
    delete[] buff;
}
