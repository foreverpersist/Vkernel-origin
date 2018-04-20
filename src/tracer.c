#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <sys/syscall.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: \n"
                       "%s <pid>\n"
                       "\n", argv[0]);
               exit(EXIT_FAILURE);
    }
    int status;
    pid_t tracee;
    long orig_rax, rax;
    int in_syscall = 0;
    long params[3]={0};

    printf("SYS_read: %d, SYS_write: %d\n", SYS_read, SYS_write);


    // tracee = atoi(argv[1]);
    tracee = fork();
    // printf("Tracee: %d\n", tracee);
    // if (ptrace(PTRACE_ATTACH, tracee, NULL, PTRACE_O_TRACESECCOMP))
    // {
    //     perror("PTRACE_ATTACH");
    //     return -1;
    // } 
    if (tracee == 0)
    {
        ptrace(PTRACE_TRACEME,0,NULL,NULL);
        execl("./test/std", "std", NULL);
    }
    else
    {

    while(1)
    {
        wait(&status);
        if (WIFEXITED(status))
            break;
        orig_rax = ptrace(PTRACE_PEEKUSER,tracee, 8*ORIG_RAX, NULL);

        // orig_rax = ptrace(PTRACE_PEEKUSER, tracee, 8 * ORIG_RAX, NULL);
        printf("The tracee made a syscall %ld\n", orig_rax);

        if(in_syscall == 0)
        {
            in_syscall = 1;
            params[0] = ptrace(PTRACE_PEEKUSER,tracee,8*RDI,NULL);
            params[1] = ptrace(PTRACE_PEEKUSER,tracee,8*RSI,NULL);
            params[2] = ptrace(PTRACE_PEEKUSER,tracee,8*RDX,NULL);
            printf("write called with %ld, %ld, %ld\n",params[0],params[1],params[2]);
        }
        else
        {
            rax = ptrace(PTRACE_PEEKUSER,tracee,8*RAX,NULL);
            printf("write returned with %ld\n",rax);
            in_syscall = 0;
        }

        ptrace(PTRACE_SYSCALL, tracee, NULL, NULL);
    }
    }

    ptrace(PTRACE_DETACH, tracee, NULL, NULL);
    return 0;
}