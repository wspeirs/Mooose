#ifndef HARNESS_CODE
#define HARNESS_CODE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

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

#include <vector.h>
			 
using k_std::vector;

class BlockDevice
{
public:
	virtual int ReadBlocks(ulong address, int blockCount, void *dest) = 0;
	virtual int WriteBlocks(ulong address, int blockCount, void *src) = 0;
	virtual ulong GetBlockSize() = 0;
	virtual ~BlockDevice() { ; }

};

#include "FileSystemBase.h"
			 

// this will simulate the disk drive
class ATADriver : public BlockDevice
{
public:
	ATADriver(char *pathToFile)
	{
		int	fd = open(pathToFile, O_RDWR);
		
		fSize = lseek(fd, 0, SEEK_END);
		lseek(fd, 0, SEEK_SET);
		
		printf("OPENED: %s\n", pathToFile);
		
		if(fd < 0)
		{
			perror("Open failed");
			exit(0);
		}
		
		// memory map the file
		start = reinterpret_cast<uchar*>(mmap(0, fSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
		
		if(start == MAP_FAILED)
		{
			perror("Map failed");
			exit(0);
		}
	}
	
	~ATADriver()
	{
		munmap(start, fSize);
	}

	int ReadBlocks(ulong address, int blockCount, void *dest)
	{
		ulong offset = BLOCK_SIZE * address;
		
		printf("\tREADING ADDR: %u\tBYTES: %u\n", offset, blockCount * BLOCK_SIZE);
		
		if(offset + (BLOCK_SIZE * blockCount) > fSize)
		{
			printf("OFFSET: %d\n", offset + (BLOCK_SIZE * blockCount));
			printf("FILE SIZE: %d\n", fSize);
			printf("ADDRESS: %d\n", address);
			printf("BLOCK COUNT: %d\n", blockCount);
			
			Panic::PrintMessage("OUT OF BOUNDS READ\n");
		}
		
		for(ulong i = 0; i < BLOCK_SIZE * blockCount; ++i)
			reinterpret_cast<uchar*>(dest)[i] = start[offset + i];
		
		return(blockCount);
	}

	int WriteBlocks(ulong address, int blockCount, void *src)
	{
		ulong offset = BLOCK_SIZE * address;
		
		if(offset + (BLOCK_SIZE * blockCount) > fSize)
			Panic::PrintMessage("OUT OF BOUNDS WRITE\n");
		
		for(ulong i = 0; i < BLOCK_SIZE * blockCount; ++i)
			start[offset + i] = reinterpret_cast<uchar*>(src)[i] ;
		
		return(blockCount);
	}
	
	ulong GetBlockSize()
	{ return(BLOCK_SIZE); }
	
	// Devices
	static const uchar DEVICE_0	= 0xE0; //0xA0;
	static const uchar DEVICE_1	= 0xF0; //0xB0;
	// Controllers
	static const ushort PRIMARY_IDE		= 0x00F0;
	static const ushort SECONDARY_IDE	= 0x0070;
	
private:
	uchar			*start;
	uint			fSize;
	static const ulong	BLOCK_SIZE = 512;

};

#endif

