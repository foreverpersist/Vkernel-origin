#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
	printf("Start init [%5d]\n", getpid());
	printf("execv %s", argv[1]);
	printf("\n");
	int ret = execv(argv[1], NULL);
	if(ret == -1)
		perror("exev error\n");
	printf("Exit init [%5d]\n", getpid());
}
