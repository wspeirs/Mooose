#include "mem_manage.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define LIST_SIZE 20

int main()
{
	// make a huge chunk of memory for the manager to use
	ulong *memory = reinterpret_cast<ulong *>(malloc(HEAP_SIZE));

	MemManager	theManager(memory);

	ulong	usedMemory = 0;
	uchar	*ptr[LIST_SIZE];
	ulong	order[LIST_SIZE];
	
	srand(time(NULL));
	
	for(int i=0; i < LIST_SIZE; ++i)
	{
		order[i] = rand()%LIST_SIZE;
//		printf("%u\n", order[i]);
	}
		
	for(int i=0; i < LIST_SIZE; ++i)
	{
		ptr[i] = new uchar[rand() % (HEAP_SIZE/LIST_SIZE - 1024)];
	}
		
	for(int i=0; i < LIST_SIZE; ++i)
	{
		if(ptr[order[i]] != NULL)
		{
			delete [] ptr[order[i]];
			ptr[order[i]] = NULL;
		}
	}
	
	theManager.WalkHeap(true); printf("\n");
	
	for(int i=0; i < LIST_SIZE; ++i)
	{
		if(ptr[i] != NULL)
		{
			delete [] ptr[i];
			ptr[i] = NULL;
		}
	}
	
	theManager.WalkHeap(true); printf("\n");
	
	free(memory);	// free the memory

	return(0);
}
