#include "demo.h"

int main(int argc,char *argv[])
{
    int fd1=open(argv[1],O_RDWR);
    int fd2=open(argv[2],O_WRONLY|O_CREAT|O_TRUNC);
    if(fd1<0||fd2<0)
    {
        cout<<"failed to open"<<endl;
        return -1;
    }
    cout<<fd1<<endl;
    char buf[1024];
    int nbytes;
    while(nbytes=read(fd1,buf,1024))
    {
        write(fd2,buf,nbytes);
    }
    return 0;
}
