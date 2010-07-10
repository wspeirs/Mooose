#include <stdio.h>
#include <unistd.h>

#define MSG "Hello from userland\n"

int main()
{
	//printf("Hello from init: %d\n", 3);
	write(1, MSG, strlen(MSG));

	return(0);
}
