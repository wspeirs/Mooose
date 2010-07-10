#ifndef HARNESS_CODE
#define HARNESS_CODE

#include <stdio.h>
#include <stdlib.h>

class AutoDisable
{
};

class Panic
{
public:
	static void HaltMachine()
	{
		printf("HALT MACHINE\n");
		exit(1);
	}

	static void PrintMessage(const char *arg, bool b = true)
	{
		(void)b;
		printf("PANIC: %s\n", arg);
		exit(1);
	}
};

#endif

