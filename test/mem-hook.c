#define _GNU_SOURCE
#include <unistd.h>
#include <dlfcn.h>
#include <stdio.h> 
#include <string.h>
#include <fcntl.h>
/*
Memory hook, which is used for memory isolation, can intercept syscalls about memory management and filter illegal memory requests.

Native syscalls about memory:

	int brk(void *addr);
		改变数据段空间的分配
	void *sbrk(intptr_t increment);
		...
	
	int mlock(const void *addr, size_t len);
		内存页面加锁
	int munlock(const void *addr, size_t len);
		内存页面解锁

	int mlockall(int flags);
		调用进程所有内存页面加锁
	int munlockall(void);
		调用进程所有内存页面解锁

	void *mmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset);
		映射虚拟内存页
	int munmap(void *addr, size_t length);
		取消虚拟内存页映射

	void *mremap(void *old_address, size_t old_size,
                    size_t new_size, int flags, ... );		
		重新映射虚拟内存地址

	int msync(void *addr, size_t length, int flags);
		将映射内存中的数据写回

	int mprotect(void *addr, size_t len, int prot);
		设置内存映像保护

	int getpagesize(void);
		获取页面大小

	void sync(void);
		将内存缓冲区数据写回

	int cacheflush(char *addr, int nbytes, int cache);
		将指定缓冲区数据写回
*/

static int (*real_brk)(void *);
static void *(*real_sbrk)(intptr_t);

static int (*real_mlock)(const void *, size_t);
static int (*real_munlock)(const void *, size_t);

static int (*real_mlockall)(int);
static int (*real_munlockall)(void);

static void *(*real_mmap)(void *, size_t, int, int, int, off_t);
static int (*real_munmap)(void *, size_t);

static void *(*real_mremap)(void *, size_t, size_t, int, ... );

static int (*real_msync)(void *, size_t, int);

static int (*real_mprotect)(void *, size_t, int);

static int (*real_getpagesize)(void);

static void (*real_sync)(void);

static int (*real_cacheflush)(char *, int, int);

int brk(void *addr)
{
	if ( !real_brk )
		real_brk = dlsym(RTLD_NEXT, __func__);
	// printf("Hack function %s\n", __func__);
	return real_brk(addr);
}

void *sbrk(intptr_t increment)
{
	// FILE *fp = fopen("hooklog", "a+");
	// fprintf(fp, "This is sbrk hook\n");
	// fclose(fp);
	if ( !real_sbrk )
                real_sbrk = dlsym(RTLD_NEXT, __func__);
	// printf("Hack function %s\n", __func__);	
	return real_sbrk(increment);
}

int mlock(const void *addr, size_t len)
{
	if ( !real_mlock )
                real_mlock = dlsym(RTLD_NEXT, __func__);
	// printf("Hack function %s\n", __func__);	
	return real_mlock(addr, len);
}

int munlock(const void *addr, size_t len)
{
	if ( !real_munlock )
                real_munlock = dlsym(RTLD_NEXT, __func__);
	// printf("Hack function %s\n", __func__);	
	return real_munlock(addr, len);
}

int mlockall(int flags)
{
	if ( !real_mlockall )
                real_mlockall = dlsym(RTLD_NEXT, __func__);
	// printf("Hack function %s\n", __func__);	
	return real_mlockall(flags);
}

int munlockall(void)
{
	if ( !real_munlockall )
                real_munlockall = dlsym(RTLD_NEXT, __func__);
	// printf("Hack function %s\n", __func__);	
	return real_munlockall();
}

void *mmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset)
{
	if ( !real_mmap )
                real_mmap = dlsym(RTLD_NEXT, __func__);
	// printf("Hack function %s\n", __func__);	
	return real_mmap(addr, length, prot, flags, fd, offset);
}

int munmap(void *addr, size_t length)
{
	if ( !real_munmap )
                real_munmap = dlsym(RTLD_NEXT, __func__);
	// printf("Hack function %s\n", __func__);	
	return real_munmap(addr, length);
}

void *mremap(void *old_address, size_t old_size,
                    size_t new_size, int flags, ... )
{
	if ( !real_mremap )
                real_mremap = dlsym(RTLD_NEXT, __func__);
	// printf("Hack function %s\n", __func__);	
	return real_mremap(old_address, old_size, new_size, flags);
}

int msync(void *addr, size_t length, int flags)
{
	if ( !real_msync )
                real_msync = dlsym(RTLD_NEXT, __func__);
	// printf("Hack function %s\n", __func__);	
	return real_msync(addr, length, flags);
}

int mprotect(void *addr, size_t len, int prot)
{
	if ( !real_mprotect )
                real_mprotect = dlsym(RTLD_NEXT, __func__);
	// printf("Hack function %s\n", __func__);	
	return real_mprotect(addr, len, prot);
}


int getpagesize(void)
{
	// FILE *fp = fopen("hooklog", "a+");
	// fprintf(fp, "This is getpagesize hook\n");
	// fclose(fp);
	if ( !real_getpagesize )
	{
                real_getpagesize = dlsym(RTLD_NEXT, __func__);
	}
	// printf("Hack function %s\n", __func__);	
	// int size = real_getpagesize();
	return real_getpagesize();
}

void sync(void)
{	
	if ( !real_sync )
                real_sync = dlsym(RTLD_NEXT, __func__);
	// printf("Hack function %s\n", __func__);	
	return real_sync();
}

int cacheflush(char *addr, int nbytes, int cache)
{
	if ( !real_cacheflush )
                real_cacheflush = dlsym(RTLD_NEXT, __func__);
	// printf("Hack function %s\n", __func__);	
	return real_cacheflush(addr, nbytes, cache);
}