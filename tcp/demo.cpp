#include "demo.h"

int main(int argc,char *argv[])
{
    int sockfd,clifd1,clifd2;
    int pfd[2];
    pid_t pid;
    sockaddr_in server_addr;
    sockaddr_in cli_addr1,cliaddr2;
    socklen_t len1,len2;
    char buff[32];
    
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0)
        return -1;

    inet_aton("0.0.0.0",&server_addr.sin_addr);
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(5001);
    if(bind(sockfd,(sockaddr *)&server_addr,sizeof(server_addr))<0)
        return -1;
    listen(sockfd,10);
    
    while(true)
    {
        clifd1=accept(sockfd,(sockaddr *)&cli_addr1,&len1);
        if(clifd1<0)
            return -1;
        if(pipe(pfd)<0)
            return -1;
        pid=fork();
        if(pid<0)
            return -1;
        else if(pid==0)
        {
            close(pfd[1]);
            pread(pfd[0],)
            send()
            close(pfd[0]);
            close(clifd1);
            exit(0);//children process terminated
        }
        close(pfd[0]);
    }
    return 0;
}