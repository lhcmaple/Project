#include "udp_client.h"

using namespace std;

int main(int argc,char *argv[])
{
    client c;
    return c.run();
}

client::client()
{
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(5000);
    inet_aton("42.192.55.215",&(servaddr.sin_addr));
    len=sizeof(servaddr);
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    cliaddr.sin_family=AF_INET;
    cliaddr.sin_port=htons(5100);
    inet_aton("0.0.0.0",&(cliaddr.sin_addr));
    bind(sockfd,(sockaddr *)&cliaddr,sizeof(cliaddr));
    buffsize=(1<<16);
    buff=new char[buffsize];
}

int client::run()
{
    int nbytes;
    while(true)
    {
        cin>>buff;
        sendto(sockfd,buff,strlen(buff),0,(sockaddr *)&servaddr,len);
        cout<<"发送成功"<<endl;
        nbytes=recvfrom(sockfd,buff,buffsize-1,0,0,0);
        buff[nbytes]='\0';
        cout<<buff<<endl;
    }
    return 0;
}

client::~client()
{
    delete[] buff;
}