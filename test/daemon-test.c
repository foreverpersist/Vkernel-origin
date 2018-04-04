#include<stdio.h>  
#include<stdlib.h>  
#include<string.h>  
#include<fcntl.h>// open  
#include<sys/types.h>  
#include<sys/stat.h>  
#include<unistd.h>  
#include<sys/wait.h>  
#include<signal.h>  
  
#define MAXFILE 65535  
  
volatile sig_atomic_t _running = 1;  
int fd;  
  
// signal handler  
void sigterm_handler(int arg)  
{  
    _running = 0;  
}  
  
int main()  
{  
    pid_t pid;  
    char *buf = "This is a Daemon, wcdj\n";  
  
    /* 屏蔽一些有关控制终端操作的信号 
     * 防止在守护进程没有正常运转起来时，因控制终端受到干扰退出或挂起 
     * */  
    signal(SIGINT,  SIG_IGN);// 终端中断  
    signal(SIGHUP,  SIG_IGN);// 连接挂断  
    signal(SIGQUIT, SIG_IGN);// 终端退出  
    signal(SIGPIPE, SIG_IGN);// 向无读进程的管道写数据  
    signal(SIGTTOU, SIG_IGN);// 后台程序尝试写操作  
    signal(SIGTTIN, SIG_IGN);// 后台程序尝试读操作  
    signal(SIGTERM, SIG_IGN);// 终止  
  
    // test  
    //sleep(20);// try cmd: ./test &; kill -s SIGTERM PID  
  
  
    // [1] fork child process and exit father process  
    pid = fork();  
    if(pid < 0)  
    {  
        perror("fork error!");  
        exit(1);  
    }  
    else if(pid > 0)  
    {  
        exit(0);  
    }  
  
    // [2] create a new session  
    setsid();  
  
    // [3] set current path  
    char szPath[1024]; 
    if(getcwd(szPath, sizeof(szPath)) == NULL)  
    {  
        perror("getcwd");  
        exit(1);  
    }  
    else  
    {  
        chdir(szPath);  
        printf("set current path succ [%s]\n", szPath);  
    }  
  
    // [4] umask 0  
    umask(0);  
  
    // [5] close useless fd  
    int i;  
    //for (i = 0; i < MAXFILE; ++i)  
    for (i = 3; i < MAXFILE; ++i)  
    {  
        close(i);  
    }  
  
    // [6] set termianl signal  
    signal(SIGTERM, sigterm_handler);  
  
    // open file and set rw limit  
    if((fd = open("outfile", O_CREAT|O_WRONLY|O_APPEND, 0600)) < 0)  
    {  
        perror("open");  
        exit(1);  
    }  
  
    pid = getpid();
    printf("\nDaemon begin to work..., and use kill -9 %d to terminate\n", pid);  
  
    // do sth in loop  
    while(_running)  
    {  
        if (write(fd, buf, strlen(buf)) != strlen(buf))  
        {  
            perror("write");  
            close(fd);  
            exit(1);  
        }  
  
        usleep(1000*1000);// 1 s  
    }  
    close(fd);  
  
  
    // print data  
    if((fd = open("outfile", O_RDONLY)) < 0)  
    {  
        perror("open");  
        exit(1);  
    }  
    char szBuf[1024] = {0};  
    if(read(fd, szBuf, sizeof(szBuf)) == -1)  
    {  
        perror("read");  
        exit(1);  
    }  
    printf("read 1024 bytes:\n%s\n", szBuf);  
  
    close(fd);  
  
    return 0;  
}  
  
/* 
   gcc -Wall -g -o test test.c 
   ps ux | grep -v grep | grep test 
   tail -f outfile 
   kill -s SIGTERM PID 
 */  
