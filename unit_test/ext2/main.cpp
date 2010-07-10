#include "ext2.h"
#include "FileSystemManager.h"

#define BUFF_SIZE 2048

const char *DISK = "disk.img";

void MemCopy(void *des, void const *src, ulong amt)
{
	memcpy(des, src, amt);
}

void memset(void *buf, uchar val, ulong size)
{
	uchar	*p = reinterpret_cast<uchar*>(buf);

	for(uint i=0; i < size; ++i)
		p[i] = val;
}

void MemSet(void *buf, uchar val, ulong size)
{
	memset(buf, val, size);
}

// protos
void RecurseFS(string path, string prefix);
void ReadTest(string path);
void ExpandFileTest(ext2 &theFs, string path);

int main()
{
//	ATADriver		myFakeDisk(const_cast<char*>(DISK));
	FileSystemManager	&fsMan = FileSystemManager::GetInstance();

	// install the ext2 file system
	//fsMan.InstallFileSystem(string("ext2"), reinterpret_cast<FileSystemFactoryT*>(new FileSystemFactory<ext2>));
	fsMan.InstallFileSystem(string("ext2"), new FileSystemFactoryT<ext2>);
	
	// mount the file system
	FileSystemManager::MountFileSystem(NULL, "/", "ext2", 0, NULL);

	// try and recurse the file system
	RecurseFS(string("/"), string(""));
	
	// try and read a file
//	ReadTest(string("/dir1/README"));
			
	// unmount the file system
	FileSystemManager::UnmountFileSystem("/");

	// remove the file system
	fsMan.RemoveFileSystem(string("ext2"));

//	ExpandFileTest(theFs, string("/README"));
//	ReadTest(theFs, string("/README"));
//	RecurseFS(theFs, string("/"), string(""));


	return(0);
}

/*
void ExpandFileTest(ext2 &theFs, string path)
{
	int	fd = theFs.Open(path, 0);

	if(fd < 0)
	{
		printf("ERROR OPENING: %s\n", path.c_str());
		return;
	}

	char	*buff = new char[BUFF_SIZE];
	int	buffSize = BUFF_SIZE;

	// set it to all a's
	for(int i=0; i < BUFF_SIZE; ++i)
		buff[i] = uchar((i%10) + 48);

// 	do
// 	{
		buffSize = BUFF_SIZE;
		buffSize = theFs.Write(fd, buff, buffSize);
// 	} while(buffSize == BUFF_SIZE);

	delete [] buff;

	theFs.Close(fd);
}

*/
void ReadTest(string path)
{
	FileSystemManager	&fsMan = FileSystemManager::GetInstance();
	
	int	fd = fsMan.Open(path, 0);

	if(fd < 0)
	{
		printf("ERROR OPENING: %s\n", path.c_str());
		return;
	}

	char	*buff = new char[BUFF_SIZE];
	int	buffSize = BUFF_SIZE;

	do
	{
		buffSize = BUFF_SIZE;
		buffSize = fsMan.Read(fd, buff, buffSize-1);
		buff[buffSize] = '\0';
		printf("%s", buff);

	} while(buffSize == BUFF_SIZE-1);

	delete [] buff;
	
	fsMan.Close(fd);
}


void RecurseFS(string path, string prefix)
{
	FileSystemManager	&fsMan = FileSystemManager::GetInstance();

	// this is like simulating the system call
	int	dd = fsMan.Open(path, 0);
	
	if(dd < 0)
	{
		printf("ERROR OPENING DIR: %s\n", path.c_str());
		return;
	}
	
	fsMan.Close(dd);
}


