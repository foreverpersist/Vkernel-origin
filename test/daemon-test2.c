#include <unistd.h>  
#include <signal.h>  
#include <stdlib.h>  
#include <string.h>  
#include <fcntl.h>  
#include <sys/stat.h>  
#include <time.h>  
#include <stdio.h>  

static int flag = 1;  
void handler(int);
  
int main()  
{  
    time_t t;  
    int fd;  
    if(-1 == daemon(0, 0))  
    {  
        printf("daemon error\n");  
        exit(1);  
    }  
    struct sigaction act;  
    act.sa_handler = handler;  
    sigemptyset(&act.sa_mask);  
    act.sa_flags = 0;  
    if(sigaction(SIGQUIT, &act, NULL))  
    {  
        printf("sigaction error.\n");  
        exit(0);  
    }  
    while(flag)  
    {  
        fd = open("outfile2", O_WRONLY | O_CREAT | O_APPEND, 0644);  
        if(fd == -1)  
        {  
            printf("open error\n");  
        }  
        t = time(0);  
        char *buf = asctime(localtime(&t));  
        write(fd, buf, strlen(buf));  
        close(fd);  
        sleep(60);  
    }  
    return 0;  
}  
void handler(int sig)  
{  
    printf("I got a signal %d\nI'm quitting.\n", sig);  
    flag = 0;  
}  
