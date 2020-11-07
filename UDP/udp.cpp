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

static const char *UDPDAT="udp.dat";

server::server(sockaddr *addr,socklen_t len)
{
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    bind(sockfd,addr,len);
    buffsize=(1<<16);
    buff=new char[buffsize];
}

int server::run()
{
    //打开udp.log日志文件
    int logfd=open("udp.log",O_WRONLY|O_APPEND|O_CREAT);
    sprintf(buff,"服务器启动...\n");
    write(logfd,buff,strlen(buff));
    //获取udp.dat下的所有目录(xx.xx),并读取对应用户的密码
    load();
    sockaddr_in cliaddr;
    socklen_t len=sizeof(cliaddr);
    size_t nbytes;
    int type;
    ushort username,password,targetname;
    while(true)
    {
        len=sizeof(cliaddr);
        nbytes=recvfrom(sockfd,buff,buffsize,0,(sockaddr *)&cliaddr,&len);
        //判定请求类型
        if(nbytes>=8)
        {
            type=gettype();
            //处理
            username=*(ushort *)buff;
            password=*(ushort *)(buff+2);
            targetname=*(ushort *)(buff+6);
            switch(type)
            {
                case 0://注册
                    if(userinfo.find(username)==userinfo.end())
                    {
                        //创建新用户
                        userinfo[username]=password;
                        buff[4]='0';
                        buff[5]='4';
                        sprintf(buff+8,"注册成功");
                        sendto(sockfd,buff,strlen(buff),0,(sockaddr *) &cliaddr,len);
                        sprintf(path,"%s/%c%c.%c%c",UDPDAT,buff[0],buff[1],buff[2],buff[3]);
                        mkdir(path,S_IRUSR|S_IWUSR|S_IXUSR);
                        //创建消息栈
                        sprintf(path1,"%s/MESSAGE.st",path);
                        close(open(path1,O_CREAT));
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
                case 1://发送
                    if(userinfo.find(username)!=userinfo.end()&&
                    userinfo.find(username)->second==password&&
                    userinfo.find(targetname)!=userinfo.end())
                    {
                        //用户名与密码验证正确,目标用户存在
                        password=userinfo.find(targetname)->second;
                        sprintf(path,"%s/%c%c.%c%c",UDPDAT,*(((char *)&targetname)),
                        *(((char *)&targetname)+1),*(((char *)&password)),*(((char *)&password)+1));
                        sprintf(path1,"%s/MESSAGE.st",path);
                        int messagefd=open(path1,O_RDWR);
                        cout<<messagefd<<endl;
                        int filelen=lseek(messagefd,0,SEEK_END);
                        if(filelen<10)//消息栈未满
                        {
                            // cout<<filelen<<endl;
                            char num='0'+filelen;
                            write(messagefd,&num,1);
                            sprintf(path1,"%s/%c",path,num);
                            cout<<path1<<endl;
                            int datafd=open(path1,O_WRONLY|O_CREAT);
                            cout<<datafd<<endl;
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
                case 2:
                    if(userinfo.find(username)!=userinfo.end()&&
                    userinfo.find(username)->second==password)
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
                    break;
            }
        }
        //写入日志文件 ip port type
        sprintf(buff,"%s %d %d\n",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port),type);
        cout<<buff<<endl;
        write(logfd,buff,strlen(buff));
    }
    return 0;
}

void server::load()
{
    //打开udp.dat目录
    DIR *dir=opendir(UDPDAT);
    if(dir==NULL)
    {
        mkdir(UDPDAT,S_IRUSR|S_IWUSR|S_IXUSR);
        dir=opendir(UDPDAT);
    }
    //读取用户名与密码
    struct dirent *dirdata;
    while(dirdata=readdir(dir))
    {
        if(strlen(dirdata->d_name)==5)
        {
            userinfo[*(ushort *)dirdata->d_name]=*(ushort *)(dirdata->d_name+3);
        }
    }
    closedir(dir);
}

int server::gettype()
{
    char num[3]{buff[4],buff[5],'\0'};
    return stoi(num);
}

server::~server()
{
    delete[] buff;
}
