#define _GNU_SOURCE
#include <unistd.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h> 

int main(int argc, char *argv[])
{
	int (*real_symbol)(void);
	char *symbol = "getpagesize";
	if( argc > 1 )
	{
		symbol = argv[1];
	}
	real_symbol = dlsym(RTLD_NEXT, symbol);
	printf("real symbol: %s\n address: %p\n", 
		symbol, real_symbol);
	int size = getpagesize();
	printf("getpagesize: %d\n", size);
}