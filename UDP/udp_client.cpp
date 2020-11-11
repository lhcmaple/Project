#include "udp_client.h"

using namespace std;

int main(int argc,char *argv[])
{
    client c(argv[1]);
    return c.run();
}

client::client(const char *servip)
{
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(5000);
    if(!servip)
        servip="127.0.0.1";
    inet_aton(servip,&(servaddr.sin_addr));
    len=sizeof(servaddr);

    buffsize=(1<<16);
    buff=new char[buffsize];
    smesgcount=0;
    rmesgcount=0;
}

int client::run()
{
    DIR *tsdat=opendir("udp.tsdat");
    dirent *dir;
    int ret;
    while(dir=readdir(tsdat))
    {
        if(dir->d_name[0]=='.')
            continue;
        for(int i=0;i<1000;++i)
        {
            usleep(rand()%100000);//模拟真实情况
            cout<<i<<endl;
            smesgcount++;
            pid_t pid=fork();
            if(pid<0)
                exit(-1);
            else if(pid==0)
            {
                sockfd=socket(AF_INET,SOCK_DGRAM,0);
                timeval tv;
                tv.tv_sec=10;
                tv.tv_usec=0;
                setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
                int nbytes;
                sprintf(buff,"udp.tsdat/%s",dir->d_name);
                int filefd=open(buff,O_RDONLY);
                nbytes=read(filefd,buff,buffsize);
                sendto(sockfd,buff,nbytes,0,(sockaddr *)&servaddr,len);
                nbytes=recvfrom(sockfd,buff,buffsize-1,0,0,0);
                if(nbytes<0)
                {
                    if(errno==EAGAIN||errno==EWOULDBLOCK)
                        printf("进程%d超时\n",getpid());
                    exit(1);//接收不到消息
                }
                else
                {
                    exit(0);
                }
            }
            else
            {
                while(waitpid(-1,&ret,WNOHANG)>0)
                {
                    if(WIFEXITED(ret)==0)
                        rmesgcount+=(WEXITSTATUS(ret)==0);
                }
            }
        }
    }
    while(waitpid(-1,&ret,0)>0)
    {
        if(WIFEXITED(ret))
            rmesgcount+=(WEXITSTATUS(ret)==0);
    }
    printf("发送%d条信息,接收%d条信息\n",smesgcount,rmesgcount);
    closedir(tsdat);
    return 0;
}

client::~client()
{
    delete[] buff;
}
