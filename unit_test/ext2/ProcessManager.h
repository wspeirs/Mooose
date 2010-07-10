#include "Singleton.h"

class ProcessManager : public Singleton<ProcessManager>
{
public:
	int InsertFileDescriptor(FileDescriptorBase* ptr)
	{
		int ret = fileDescriptors.size();
		
		fileDescriptors.push_back(ptr);
		
		return ret;
	}
	
	inline FileDescriptorBase *GetFileDescriptor(int fd)
	{ return fileDescriptors[fd]; }
	
	void RemoveFileDescriptor(int fd)
	{
		if(uint(fd) > fileDescriptors.size()-1)
			return;
		
		fileDescriptors.erase(fileDescriptors.begin() + fd);
	}
	
private:
	vector<FileDescriptorBase*>	fileDescriptors;
};

