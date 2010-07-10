#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	pid_t	pid;

	if((pid = fork()) == 0)
	{
		printf("CHILD\n");
	}

	else
	{
		printf("PARENT: CHILD = %d\n", pid);
	}
	
	return(0);
}
