#include "demo.h"

using std::cout;
using std::endl;

int gshmfd=-1;

void ae()
{
    if(gshmfd>0)
        shmctl(gshmfd,IPC_RMID,NULL);
}

int main(int argc,char *argv[])
{
    int sockfd,clifd1,clifd2;
    int pfd[2];
    pid_t pid;
    sockaddr_in server_addr;
    sockaddr_in cli_addr1,cli_addr2;
    socklen_t len1,len2;
    char buff[32],nbytes;
    int shmfd=shmget(IPC_PRIVATE,1,IPC_CREAT|0600);
    char *flag=(char *)shmat(shmfd,NULL,0);

    gshmfd=shmfd;
    atexit(&ae);
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
        *flag=0;
        clifd1=accept(sockfd,(sockaddr *)&cli_addr1,&len1);
        if(clifd1<0)
            return 1;
        if(pipe(pfd)<0)
            return 2;
        pid=fork();
        if(pid<0)
            return 3;
        else if(pid==0)
        {
            nbytes=read(pfd[0],buff,32);
            *flag=1;
            if(nbytes>0)
                send(clifd1,buff,nbytes,0);

            sprintf(buff,"%s %d",inet_ntoa(cli_addr1.sin_addr),ntohs(cli_addr1.sin_port));
            cout<<buff<<endl;
            write(pfd[1],buff,strlen(buff));
            close(pfd[0]);
            close(pfd[1]);
            close(clifd1);
            exit(0);//children process terminated
        }

        close(clifd1);
        clifd2=accept(sockfd,(sockaddr *)&cli_addr2,&len2);
        if(clifd2<0)
            return 4;
        sprintf(buff,"%s %d",inet_ntoa(cli_addr2.sin_addr),ntohs(cli_addr2.sin_port));
        cout<<buff<<endl;
        write(pfd[1],buff,strlen(buff));

        while(!*flag)
            usleep(1000);
        nbytes=read(pfd[0],buff,32);
        if(nbytes>0)
            send(clifd2,buff,nbytes,0);

        close(pfd[0]);
        close(pfd[1]);
        close(clifd2);
        wait(NULL);
    }

    shmdt(flag);
    return 0;
}