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

    buff=new char[NPOOL][BUFFSIZE];

    userinfo=new char[USERNUM][3];
    memset(userinfo,0,USERNUM*3);

    usermutex=new pthread_mutex_t[65536];
    for(int i=0;i<USERNUM;++i)
        pthread_mutex_init(usermutex+i,NULL);

    servsem=new sem_t[NPOOL];
    clisem=new sem_t[NPOOL];
    for(int i=0;i<NPOOL;++i)
    {
        sem_init(servsem+i,0,0);
        sem_init(clisem+i,0,1);
    }
}

int server::run()
{
    mode_t premode=umask(0);

    //打开udp.log日志文件
    logfd=open("udp.log",O_WRONLY|O_APPEND|O_CREAT,RWRWRW);
    sprintf(buff[0],"服务器启动...\n");
    write(logfd,buff[0],strlen(buff[0]));

    //获取udp.dat下的所有目录(xx.xx),并读取对应用户的密码
    load();
    //创建线程池
    for(int i=0;i<NPOOL;++i)
    {
        args[i].s=this;
        args[i].id=i;
        pthread_t ptid;
        pthread_create(&ptid,NULL,pthread_func,(void *)(args+i));
    }
    //并发处理请求
    while(true)
    {
        for(int i=0;i<NPOOL;++i)
        {
            if(sem_trywait(clisem+i))
                continue;
            len[i]=sizeof(cliaddr[i]);
            nbytes[i]=recvfrom(sockfd,buff[i],BUFFSIZE,0,(sockaddr *)(cliaddr+i),len+i);
            sem_post(servsem+i);
        }
    }

    close(logfd);
    umask(premode);
    return 0;
}

void server::worker(int id)
{
    char path[PATH_LENGTH],path1[PATH_LENGTH];
    int type;

    while(true)
    {
        sem_wait(servsem+id);
        username[id]=*(ushort *)buff[id];
        password[id]=*(ushort *)(buff[id]+2);
        targetname[id]=*(ushort *)(buff[id]+6);
        type=gettype(id);
        while(pthread_mutex_lock(usermutex+id));
        switch(type)
        {
            case 0://注册
                if(userinfo[username[id]][2]==0)
                {
                    //创建新用户
                    userinfo[username[id]][2]=1;
                    *((ushort *) (userinfo[username[id]]))=password[id];
                    buff[id][4]='0';
                    buff[id][5]='4';
                    sprintf(buff[id]+8,"注册成功");
                    sendto(sockfd,buff[id],strlen(buff[id]),0,(sockaddr *) &cliaddr+id,len[id]);
                    sprintf(path,"%s/%c%c.%c%c",UDPDAT,buff[id][0],buff[id][1],buff[id][2],buff[id][3]);
                    mkdir(path,RWXRWRW);
                    //创建消息栈
                    sprintf(path1,"%s/MESSAGE.st",path);
                    close(open(path1,O_CREAT,RWRWRW));
                }
                else
                {
                    //用户已存在
                    buff[id][4]='0';
                    buff[id][5]='3';
                    sprintf(buff[id]+8,"注册失败");
                    sendto(sockfd,buff[id],strlen(buff[id]),0,(sockaddr *) cliaddr+id,len[id]);
                }
                break;
            case 1://发送,need to be add somethings
                if(userinfo[username[id]][2]>0&&
                *((ushort *) (userinfo[username[id]]))==password[id]&&
                userinfo[targetname[id]][2]>0)
                {
                    //用户名与密码验证正确,目标用户存在
                    password[id]=*((ushort *) (userinfo[targetname[id]]));
                    sprintf(path,"%s/%c%c.%c%c",
                        UDPDAT,
                        *(((char *)&targetname[id])),
                        *(((char *)&targetname[id])+1),
                        *(((char *)&password[id])),
                        *(((char *)&password[id])+1));
                    sprintf(path1,"%s/MESSAGE.st",path);
                    int messagefd=open(path1,O_RDWR);
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
                        buff[id][6]=buff[id][0];
                        buff[id][7]=buff[id][1];
                        write(datafd,buff[id]+6,nbytes[id]-6);
                        close(datafd);

                        buff[id][4]='0';
                        buff[id][5]='6';
                        buff[id][6]='0';
                        buff[id][7]='0';
                        sprintf(buff[id]+8,"发送成功");
                        sendto(sockfd,buff[id],strlen(buff[id]),0,(sockaddr *) cliaddr+id,len[id]);
                    }
                    else
                    {
                        buff[id][4]='1';
                        buff[id][5]='0';
                        sprintf(buff[id]+8,"消息栈满");
                        sendto(sockfd,buff[id],strlen(buff[id]),0,(sockaddr *) cliaddr+id,len[id]);
                    }
                    close(messagefd);
                }
                else
                {
                    buff[id][4]='0';
                    buff[id][5]='5';
                    sprintf(buff[id]+8,"未注册或目标用户未注册");
                    sendto(sockfd,buff[id],strlen(buff[id]),0,(sockaddr *) cliaddr+id,len[id]);
                }
                break;
            case 2://need to be add somethings
                if(userinfo[username[id]][2]>0&&
                *((ushort *) (userinfo[username[id]]))==password[id])
                {
                    //用户名与密码验证正确
                    sprintf(path,"%s/%c%c.%c%c",UDPDAT,buff[id][0],buff[id][1],buff[id][2],buff[id][3]);
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
                        buff[id][4]='0';
                        buff[id][5]='8';
                        nbytes[id]=read(datafd,buff[id]+6,BUFFSIZE-6);
                        close(datafd);
                        sendto(sockfd,buff[id],nbytes[id]+6,0,(sockaddr *) cliaddr+id,len[id]);
                        remove(path1);
                    }
                    else
                    {
                        //无消息待转发
                        buff[id][4]='0';
                        buff[id][5]='7';
                        sprintf(buff[id]+8,"消息栈空");
                        sendto(sockfd,buff[id],strlen(buff[id]),0,(sockaddr *) cliaddr+id,len[id]);
                    }
                    close(messagefd);
                }
                else
                {
                    buff[id][4]='0';
                    buff[id][5]='9';
                    sprintf(buff[id]+8,"密码错误");
                    sendto(sockfd,buff[id],strlen(buff[id]),0,(sockaddr *) cliaddr+id,len[id]);
                }
                break;
            default:
                sprintf(buff[id]+8,"无此服务代码");
                sendto(sockfd,buff[id],strlen(buff[id]),0,(sockaddr *) cliaddr+id,len[id]);
                break;
        }
        pthread_mutex_unlock(usermutex+id);

        //写入日志文件,格式:ip-port user type
        sprintf(buff[id],"(%s-%5d):%02d\n",inet_ntoa(cliaddr[id].sin_addr),ntohs(cliaddr[id].sin_port),type);
        while(pthread_mutex_lock(&logmutex));
        write(logfd,buff[id],strlen(buff[id]));
        pthread_mutex_unlock(&logmutex);

        sem_post(clisem+id);
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
        if(strlen(dirdata->d_name)==5&&dirdata->d_name[2]=='.')
        {
            *(ushort *)(userinfo[*(ushort *)dirdata->d_name])=*(ushort *)(dirdata->d_name+3);
            userinfo[*(ushort *)dirdata->d_name][2]=1;
        }
    }
    closedir(dir);
}

int server::gettype(int id)
{
    if(ISNUMBER(buff[id][4])&&ISNUMBER(buff[id][5]))
        return TONUMBER(buff[id][4],buff[id][5]);
    else
        return -1;
}

void *pthread_func(void *arg)
{
    server *s=((pthread_arg *)arg)->s;
    int id=((pthread_arg *)arg)->id;
    s->worker(id);
}

server::~server()
{
    delete[] buff;
    delete[] userinfo;
    for(int i=0;i<USERNUM;++i)
    {
        pthread_mutex_destroy(usermutex+i);
    }
    delete[] usermutex;
    for(int i=0;i<NPOOL;++i)
    {
        sem_destroy(servsem+i);
        sem_destroy(clisem+i);
    }
    delete[] servsem;
    delete[] clisem;
}
