#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>

typedef pid_t (*FORK)(void);

typedef int (*GETPAGESIZE)(void);

pid_t fork(void)
{
	static void *handle = NULL;
	static FORK old = NULL;

	if(!handle)
	{
		handle = dlopen("libc.so.6", RTLD_LAZY);
		old = (FORK)dlsym(handle, "fork");
	}

	printf("oops!!! hack function %s - %p invoked.\n", __func__, old);
	return old();
}

int getpagesize(void)
{
	static void *handle = NULL;
	static GETPAGESIZE old = NULL;
	if(!handle)
	{
		handle = dlopen("libc.so.6", RTLD_LAZY);
		old = (GETPAGESIZE)dlsym(handle, "getpagesize");
	}
	printf("Hack function %s\n", __func__);	
	return old();

}