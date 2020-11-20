#include "udp.h"

int main(int argc,char *argv[])
{
    struct sockaddr_in servaddr;
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(5000);
    inet_aton("0.0.0.0",&(servaddr.sin_addr));
    server s((sockaddr*) &servaddr,sizeof(servaddr));
    return s.run();
}

const char *server::UDPDAT="udp.dat";

server::server(sockaddr *addr,socklen_t len)
{
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    bind(sockfd,addr,len);
    buffsize=(1<<16);
    buff=new char[buffsize];
}

int server::run()
{
    mode_t premode=umask(0);
    //打开udp.log日志文件
    int logfd=open("udp.log",O_WRONLY|O_APPEND|O_CREAT,RWRWRW);
    sprintf(buff,"服务器启动...\n");
    write(logfd,buff,strlen(buff));
    //获取udp.dat下的所有目录(xx.xx),并读取对应用户的密码
    int shfd=shmget(ftok("udp",0),65536*3,IPC_CREAT|0666);
    userinfo=(TYPE_USERINFO) shmat(shfd,0,0);
    memset(userinfo,0,65536*3);
    load();

    //创建进程池
    for(int i=0;i<NPOOL;++i)
    {
        pipe(pfd[i]);
        pool[i]=fork();
        if(pool[i]<0)
            return -1;
        else if(pool[i]==0)
        {
            close(pfd[i][1]);
            worker(pfd[i][0]);
            exit(0);
        }
        else
        {
            close(pfd[i][0]);
        }
    }

    //并发处理请求
    int nextprocess=0;
    size_t nbytes;
    while(true)
    {
        len=sizeof(cliaddr);
        nbytes=recvfrom(sockfd,buff,buffsize,0,(sockaddr *)&cliaddr,&len);
        write(pfd[nextprocess][1],&len,sizeof(len));
        write(pfd[nextprocess][1],&cliaddr,len);
        write(pfd[nextprocess][1],&nbytes,sizeof(nbytes));
        write(pfd[nextprocess][1],buff,nbytes);
        nextprocess=(nextprocess+1)%NPOOL;

        //写入日志文件,格式:ip-port user type
        sprintf(buff,"%s-%d\n",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));
        write(logfd,buff,strlen(buff));
        // for(int i=0;i<65536;++i)
        //     if(userinfo[i][2]!=0||userinfo[i][1]!=0||userinfo[i][0]!=0)
        //     {
        //         cout<<i<<"*"<<userinfo[i][0]<<userinfo[i][1]<<(int)userinfo[i][2]<<endl;
        //     }
        // cout<<endl;
    }

    close(logfd);
    umask(premode);
    return 0;
}

void server::worker(int rfd)
{
    //需要增加文件锁与消息发送锁,并通知父进程修改userinfo
    size_t nbytes;
    int type;
    while(true)
    {
        read(rfd,&len,sizeof(len));
        read(rfd,&cliaddr,len);
        read(rfd,&nbytes,sizeof(nbytes));
        read(rfd,buff,nbytes);
        if(nbytes<8)
            continue;
        type=gettype();
        username=*(ushort *)buff;
        password=*(ushort *)(buff+2);
        targetname=*(ushort *)(buff+6);
        switch(type)
        {
            case 0://注册
                if(userinfo[username][2]==0)
                {
                    //创建新用户
                    userinfo[username][2]=1;
                    *((ushort *) (userinfo[username]))=password;
                    buff[4]='0';
                    buff[5]='4';
                    sprintf(buff+8,"注册成功");
                    sendto(sockfd,buff,strlen(buff),0,(sockaddr *) &cliaddr,len);
                    sprintf(path,"%s/%c%c.%c%c",UDPDAT,buff[0],buff[1],buff[2],buff[3]);
                    mkdir(path,RWXRWRW);
                    //创建消息栈
                    sprintf(path1,"%s/MESSAGE.st",path);
                    close(open(path1,O_CREAT,RWRWRW));
                }
                else
                {
                    //用户已存在
                    buff[4]='0';
                    buff[5]='3';
                    sprintf(buff+8,"注册失败");
                    sendto(sockfd,buff,strlen(buff),0,(sockaddr *) &cliaddr,len);
                }
                break;
            case 1://发送,need to be add somethings
                if(userinfo[username][2]>0&&
                *((ushort *) (userinfo[username]))==password&&
                userinfo[targetname][2]>0)
                {
                    //用户名与密码验证正确,目标用户存在
                    password=*((ushort *) (userinfo[targetname]));
                    sprintf(path,"%s/%c%c.%c%c",UDPDAT,*(((char *)&targetname)),
                    *(((char *)&targetname)+1),*(((char *)&password)),*(((char *)&password)+1));
                    sprintf(path1,"%s/MESSAGE.st",path);
                    int messagefd=open(path1,O_RDWR);
                    // cout<<messagefd<<endl;
                    int filelen=lseek(messagefd,0,SEEK_END);
                    if(filelen<10)//消息栈未满
                    {
                        // cout<<filelen<<endl;
                        char num='0'+filelen;
                        write(messagefd,&num,1);
                        sprintf(path1,"%s/%c",path,num);
                        // cout<<path1<<endl;
                        int datafd=open(path1,O_WRONLY|O_CREAT,RWRWRW);
                        // cout<<datafd<<endl;
                        buff[6]=buff[0];
                        buff[7]=buff[1];
                        write(datafd,buff+6,nbytes-6);
                        close(datafd);

                        buff[4]='0';
                        buff[5]='6';
                        buff[6]='0';
                        buff[7]='0';
                        sprintf(buff+8,"发送成功");
                        sendto(sockfd,buff,strlen(buff),0,(sockaddr *) &cliaddr,len);
                    }
                    else
                    {
                        buff[4]='1';
                        buff[5]='0';
                        sprintf(buff+8,"消息栈满");
                        sendto(sockfd,buff,strlen(buff),0,(sockaddr *) &cliaddr,len);
                    }
                    close(messagefd);
                }
                else
                {
                    buff[4]='0';
                    buff[5]='5';
                    sprintf(buff+8,"未注册或目标用户未注册");
                    sendto(sockfd,buff,strlen(buff),0,(sockaddr *) &cliaddr,len);
                }
                break;
            case 2://need to be add somethings
                if(userinfo[username][2]>0&&
                *((ushort *) (userinfo[username]))==password)
                {
                    //用户名与密码验证正确
                    sprintf(path,"%s/%c%c.%c%c",UDPDAT,buff[0],buff[1],buff[2],buff[3]);
                    sprintf(path1,"%s/MESSAGE.st",path);
                    int messagefd=open(path1,O_RDWR);
                    int filelen=lseek(messagefd,0,SEEK_END);
                    if(filelen>0)
                    {
                        //有消息待转发
                        lseek(messagefd,-1,SEEK_END);
                        char num;
                        read(messagefd,&num,1);
                        ftruncate(messagefd,filelen-1);
                        close(messagefd);
                        sprintf(path1,"%s/%c",path,num);
                        int datafd=open(path1,O_RDONLY);
                        buff[4]='0';
                        buff[5]='8';
                        nbytes=read(datafd,buff+6,buffsize-6);
                        close(datafd);
                        sendto(sockfd,buff,nbytes+6,0,(sockaddr *) &cliaddr,len);
                        remove(path1);
                    }
                    else
                    {
                        //无消息待转发
                        buff[4]='0';
                        buff[5]='7';
                        sprintf(buff+8,"消息栈空");
                        sendto(sockfd,buff,strlen(buff),0,(sockaddr *) &cliaddr,len);
                    }
                    close(messagefd);
                }
                else
                {
                    buff[4]='0';
                    buff[5]='9';
                    sprintf(buff+8,"密码错误");
                    sendto(sockfd,buff,strlen(buff),0,(sockaddr *) &cliaddr,len);
                }
                break;
            default:
                sprintf(buff+8,"无此服务代码");
                sendto(sockfd,buff,strlen(buff),0,(sockaddr *) &cliaddr,len);
                break;
        }
    }
}

void server::load()
{
    //打开udp.dat目录
    DIR *dir=opendir(UDPDAT);
    if(dir==NULL)
    {
        mkdir(UDPDAT,RWXRWRW);
        dir=opendir(UDPDAT);
    }
    //读取用户名与密码
    struct dirent *dirdata;
    while(dirdata=readdir(dir))
    {
        if(strlen(dirdata->d_name)==5)
        {
            *(ushort *)(userinfo[*(ushort *)dirdata->d_name])=*(ushort *)(dirdata->d_name+3);
            userinfo[*(ushort *)dirdata->d_name][2]=1;
        }
    }
    closedir(dir);
}

int server::gettype()
{
    char num[3]{buff[4],buff[5],'\0'};
    if(buff[4]>='0'&&buff[4]<='9'&&buff[5]>='0'&&buff[5]<='9')
        return stoi(num);
    else
        return -1;
}

server::~server()
{
    delete[] buff;
}
