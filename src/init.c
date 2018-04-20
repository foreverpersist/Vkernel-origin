// SPDX-License-Identifier: GPL-2.0
/*
 * Seccomp filter example for x86 (32-bit and 64-bit) with BPF macros
 *
 * Copyright (c) 2012 The Chromium OS Authors <chromium-os-dev@chromium.org>
 * Author: Will Drewry <wad@chromium.org>
 *
 * The code may be used by anyone for any purpose,
 * and can serve as a starting point for developing
 * applications using prctl(PR_SET_SECCOMP, 2, ...).
 */
#if defined(__i386__) || defined(__x86_64__)
#define SUPPORTED_ARCH 1
#endif

#if defined(SUPPORTED_ARCH)
#define __USE_GNU 1
#define _GNU_SOURCE 1

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <string.h>
#include <linux/unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <signal.h>

#include "jsmn.h"
#include "hashmap.h"

// Maybe it is not stable
#define __NRCALL_FILE "/usr/include/asm-generic/unistd.h"

#define syscall_arch (offsetof(struct seccomp_data, arch))
#define syscall_nr (offsetof(struct seccomp_data, nr))
#define syscall_arg(_n) (offsetof(struct seccomp_data, args[_n]))

#if defined(__i386__)
#define REG_RESULT  REG_EAX
#define REG_SYSCALL REG_EAX
#define REG_ARG0    REG_EBX
#define REG_ARG1    REG_ECX
#define REG_ARG2    REG_EDX
#define REG_ARG3    REG_ESI
#define REG_ARG4    REG_EDI
#define REG_ARG5    REG_EBP
#elif defined(__x86_64__)
#define REG_RESULT  REG_RAX
#define REG_SYSCALL REG_RAX
#define REG_ARG0    REG_RDI
#define REG_ARG1    REG_RSI
#define REG_ARG2    REG_RDX
#define REG_ARG3    REG_R10
#define REG_ARG4    REG_R8
#define REG_ARG5    REG_R9
#endif

#ifndef PR_SET_NO_NEW_PRIVS
#define PR_SET_NO_NEW_PRIVS 38
#endif

#ifndef SYS_SECCOMP
#define SYS_SECCOMP 1
#endif

HASHMAP_DEFINE(str, const char *, int)

hashmap(str) load_syscall_map(char *names[], int nrs[], int size)
{
    int i;
    hashmap(str) map = hashmap_create(str, 0, 0);
    hashmap_set_hash_func(str, map, str_hash);
    hashmap_set_compare_func(str, map, strcmp);
    hashmap_set_key_funcs(str, map, str_key_alloc, str_key_free);

    for (i = 0; i < size; i++)
    {
        hashmap_put(str, map, names[i], nrs[i]);
    }
    return map;
}

int parse_config(const char *path, int **p_syscalls, int *p_size)
{
    FILE *fp;
    char *buf;
    jsmn_parser p;
    jsmntok_t t[128];
    long length;
    size_t r;
    char key[128];
    char value[128];
    int nr;
    int i;
    if ((fp = fopen(path, "r")) == NULL)
    {
        perror("fail to open config file");
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    buf = (char *) malloc(sizeof(char) * length);
    if (buf == NULL)
    {
        perror("memory error");
        return 22;
    }
    rewind(fp);
    if ((r = fread(buf, 1, length, fp)) != length)
    {
        perror("read error");
        return -3;
    }
    jsmn_init(&p);
    r = jsmn_parse(&p, buf, length, t, sizeof(t) / sizeof(t[0]));
    *p_size = r / 2;
    *p_syscalls = (int *) malloc(sizeof(int) * (*p_size));

    for (i = 1; i < r; i++)
    {
        strncpy(key, buf + t[i].start, t[i].end - t[i].start);
        strncpy(value, buf + t[i + 1].start, t[i + 1].end - t[i + 1].start);
        nr = strtol(value, NULL, 10);
        (*p_syscalls)[i / 2] = nr;
        i++;
    }

    return 0;
}

static void emulator(int nr, siginfo_t *info, void *void_context)
{
    ucontext_t *ctx = (ucontext_t *)(void_context);
    int syscall;
    char *buf;
    ssize_t bytes;
    size_t len;
    write(STDOUT_FILENO, "[emulator] ", 11);
    if (info->si_code != SYS_SECCOMP)
        return;
    if (!ctx)
        return;
    syscall = ctx->uc_mcontext.gregs[REG_SYSCALL];
    buf = (char *) ctx->uc_mcontext.gregs[REG_ARG1];
    len = (size_t) ctx->uc_mcontext.gregs[REG_ARG2];

    if (syscall != __NR_write)
        return;
    if (ctx->uc_mcontext.gregs[REG_ARG0] != STDERR_FILENO)
        return;
    /* Redirect stderr messages to stdout. Doesn't handle EINTR, etc */
    ctx->uc_mcontext.gregs[REG_RESULT] = -1;
    if (write(STDOUT_FILENO, "[ERR] ", 6) > 0) {
        bytes = write(STDOUT_FILENO, buf, len);
        ctx->uc_mcontext.gregs[REG_RESULT] = bytes;
    }
    return;

}

static int install_emulator()
{
    struct sigaction act;
    sigset_t mask;
    memset(&act, 0, sizeof(act));
    sigemptyset(&mask);
    sigaddset(&mask, SIGSYS);

    act.sa_sigaction = &emulator;
    act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSYS, &act, NULL) < 0) {
        perror("sigaction");
        return -1;
    }
    if (sigprocmask(SIG_UNBLOCK, &mask, NULL)) {
        perror("sigprocmask");
        return -1;
    }
    return 0;
}

static int install_filter(int t_arch, int *syscalls, int size)
{
    int len = 2 * size + 5;
    struct sock_filter filter[len];

    /* [0] Load architecture from 'seccomp_data' buffer into
           accumulator */
    filter[0] = (struct sock_filter) BPF_STMT(BPF_LD | BPF_W | BPF_ABS, syscall_arch);

    /* [1] Jump forward X(=2*size+2) instructions to the last instruction  
           if architecture does not match 't_arch' */ 
    filter[1] = (struct sock_filter) BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, t_arch, 0, 2 * size + 2);

    /* [2] Load system call number from 'seccomp_data' buffer into
           accumulator */
    filter[2] = (struct sock_filter) BPF_STMT(BPF_LD | BPF_W | BPF_ABS, syscall_nr);

    /* [3] Trace specified syscalls in a loop*/
    int i;
    int nr;
    for (i = 0; i < size; i++)
    {
        nr = syscalls[i];
        printf("trace syscall %d\n", nr);
        filter[3 + 2 * i]= (struct sock_filter) BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, (nr), 0, 1);
        filter[3 + 2 * i + 1] = (struct sock_filter) BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_TRAP);
    }

    /* [5] Destination of system call number mismatch: allow other
           system calls */
    filter[len-2] = (struct sock_filter) BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_ALLOW);

    /* [6] Destination of architecture mismatch: kill task */
    filter[len-1] = (struct sock_filter) BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL);

    struct sock_fprog prog = {
        .len = len,
        .filter = filter,
    };

    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog)) {
        perror("seccomp");
        return 1;
    }

    return 0;
}

static int print_syscall(const char *name, int nr, void *args)
{
    printf("[syscall] name: %s,  nr: %d\n", name, nr);
    return 0;
}

int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        fprintf(stderr, "Usage: \n"
                       "%s <config> <prog> [<args>]\n"
                       "Hint for <arch>: AUDIT_ARCH_I386: 0x%X\n"
                       "                 AUDIT_ARCH_X86_64: 0x%X\n"
                       "\n", argv[0], AUDIT_ARCH_I386, AUDIT_ARCH_X86_64);
               exit(EXIT_FAILURE);
    }

    pid_t pid = getpid();
    pid_t ppid = getppid();
    printf("pid: %d\n", pid);
    printf("ppid: %d\n", ppid);

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
    {
        perror("prctl");
        exit(EXIT_FAILURE);
    }

    char * names[] = {"fork", "clone", "execve", "exit", "vfork", 
        "open", "creat", "close", "read", "write", 
        "mkdir", "mknod", "rmdir", "link", "symlink", "unlink", "mount", "unmount", 
        "swapon", "swapoff", "create_module", "delete_module", "init_module", 
        "brk", "mlock", "munlock", "mlockall", "munlockall", "mmap", "munmap", 
        "mremap", "msync", "mprotect", "sync", 
        "socket", "bind", "connect", "socketpair", 
        "pipe", 
        "semctl", "semget", "semop", 
        "shmctl", "shmget", "shmat", "shmdt"};
    int nrs[]= {__NR_fork, __NR_clone, __NR_execve, __NR_exit, __NR_vfork, 
        __NR_open, __NR_creat, __NR_close, __NR_read, __NR_write, 
        __NR_mkdir, __NR_mknod, __NR_rmdir, __NR_link, __NR_symlink, __NR_unlink, __NR_mount, __NR_umount2, 
        __NR_swapon, __NR_swapoff, __NR_create_module, __NR_delete_module, __NR_init_module, 
        __NR_brk, __NR_mlock, __NR_munlock, __NR_mlockall, __NR_munlock, __NR_mmap, __NR_munmap, 
        __NR_mremap, __NR_msync, __NR_mprotect, __NR_sync, 
         __NR_socket, __NR_bind, __NR_connect, __NR_socketpair, 
        __NR_pipe, 
        __NR_semctl, __NR_semget, __NR_semop, 
        __NR_shmctl, __NR_shmget, __NR_shmat, __NR_shmdt};

    hashmap(str) map = load_syscall_map(names, nrs, sizeof(nrs) / sizeof(nrs[0]));
    hashmap_foreach(str, map, print_syscall, NULL);

    int *syscalls;
    int size;
    if (parse_config(argv[1], &syscalls, &size))
    {
        perror("parse_config");
        exit(EXIT_FAILURE);
    }

    if (install_emulator())
    {
        perror("install_emulator");
        exit(EXIT_FAILURE);
    }

    if (install_filter(AUDIT_ARCH_X86_64, syscalls, size))
    {
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "Test stderr\n");

    execv(argv[2], &argv[2]);
    perror("execv");
    exit(EXIT_FAILURE);
    return 0;
}
#else   /* SUPPORTED_ARCH */
/*
 * This sample is x86-only.  Since kernel samples are compiled with the
 * host toolchain, a non-x86 host will result in using only the main()
 * below.
 */
int main(void)
{
    return 1;
}
#endif  /* SUPPORTED_ARCH */
