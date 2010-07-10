#include <VirtualConsoleManager.h>
#include <string.h>

using k_std::string;

int syscall_write(int fd, void *data, int size)
{
	// 0 = STDOUT & 1 = STDERR
	if(fd != 0 && fd != 1)	// we don't deal with this yet
		Panic::PrintMessage("Unknown file descriptor\n");
	
//	VirtualConsole *curConsole = VirtualConsoleManager::GetInstance()->GetCurrentConsole();
	
//	curConsole->printf("%s", data);
	
	// make a string from our data
	string	tmp(reinterpret_cast<char*>(data), size);
	
	printf("%s", tmp.c_str());
	
	return(size);
}


int syscall_read(int fd, void *data, int size)
{
	return(0);
}
