/*
 * Copyright (c) 2005
 * William R. Speirs
 *
 * Permission to use, copy, distribute, or modify this software for
 * the purpose of education is herby granted without fee. Permission
 * to sell this software or its documentation is hereby denied without
 * first obtaining the written consent of the author. In all cases, the 
 * above copyright notice must appear and this permission notice must 
 * appear in the supporting documentation. William R. Speirs makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */


/** @file test.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <ProcessManager.h>
#include <Semaphore.h>
#include <FileSystemManager.h>
#include <mem_utils.h>
#include <Partition.h>
#include <Ext2.h>
#include <VirtualConsoleManager.h>

#include <vector.h>
#include <string.h>
#include <algorithms.h>

using k_std::vector;
using k_std::string;


string GetLine(VirtualConsole *theConsole)
{
	string	retString;
	uchar	tmpChar;
	
	while(1)
	{
		tmpChar = theConsole->GetCharacter();
		
		if(tmpChar == 0x8)	// backspace
		{
			if(retString.size() > 0)
			{
				theConsole->EraseLastCharacter();	// erase the character
				retString.pop_back();
			}
			continue;
		}
		
		theConsole->printf("%c", tmpChar);
		
		if(tmpChar == '\n')
			break;		
		
		retString.push_back(tmpChar);
	}
	
	return(retString);
}

string ParsePath(string curDir, string path)
{
	list<string>	pathDirs, curDirs;
	string		ret("/");
	
	if(path.BeginsWith("/"))	// absolute path
		curDir = "/";
	
	path.Tokenize('/', pathDirs);	// split up the path
	curDir.Tokenize('/', curDirs);	// split up the current directory
	
	// go through and remove all the .. & . stuff
	for(list<string>::iterator cur = pathDirs.begin(); cur != pathDirs.end(); ++cur)
	{
		if(*cur == "..")
		{
			if(curDirs.size() != 0)
				curDirs.erase(--curDirs.end());	// remove the last item
		}
		
		else if(*cur == ".")		// move on
			continue;
		
		else	// just copy this dir on
			curDirs.push_back(*cur);
	}
	
	// put the string back together
	for(list<string>::iterator it = curDirs.begin(); it != curDirs.end(); ++it)
	{
		ret += *it;
		ret += "/";
	}
	
	ret.erase(--ret.end());	// no trailing /
	
	if(ret.size() == 0)
		ret = "/";
	
	return ret;
}

void ShellMain(void *arg)
{
	(void)arg;
	
	VirtualConsole		*myConsole = VirtualConsoleManager::GetInstance().GetCurrentConsole();
	FileSystemManager	&fsMan = FileSystemManager::GetInstance();
	
	uchar	tmp = myConsole->GetCharacter(); (void)tmp;	// need this to "get things going"
	myConsole->ClearScreen();	// clear the screen
	
	// print a little welcome
	myConsole->SetAttribute(VirtualConsole::FG_GREEN);
	myConsole->printf("*** Welcome to MOOOSE ***\n\n");
	myConsole->SetAttribute(VirtualConsole::FG_WHITE);
	
	string				curDir("/");
	int				curDirDes = fsMan.Open(curDir.c_str(), 0);
	
	if(curDirDes < 0)
		PANIC("Error mounting file system\n");
	
	while(1)	// this is the main loop for the shell
	{
		myConsole->printf("[%s] $ ", curDir.c_str());	// command line
		
		string	input = GetLine(myConsole);	// get the command
		
		if(input == "ls")	// want to read a directory
		{
//			fsMan.Read(curDirDes, dirList);
		
			;
		}
		
		else if(input.BeginsWith("cd"))		// change directories
		{
			vector<string>	command;
			string		tmpDir;
			
			input.Tokenize(' ', command);
			
			// parse the string
			tmpDir = ParsePath(curDir, command[1]);
			
			int	tmpDes = fsMan.Open(tmpDir.c_str(), 0);
			
			if(tmpDes < 0)
				myConsole->printf("Error opening directory\n");
		
			else
			{
				fsMan.Close(curDirDes);
				curDirDes = tmpDes;
				
				curDir = tmpDir;
			}
		}
		
		else if(input.BeginsWith("more"))
		{
			// parse the string
			vector<string>	command;
			string		fileAndPath;
			
			input.Tokenize(' ', command);
			
			// parse the string
			fileAndPath = ParsePath(curDir, command[1]);
			
			int	fd = fsMan.Open(fileAndPath.c_str(), 0);
			
			if(fd < 0)
			{
				myConsole->printf("Error opening file: %d\n", fd);
				continue;
			}
			
			int	amtRead = 1023;
			char	*buff = new char[1024];
			
			while(amtRead == 1023)
			{
				amtRead = fsMan.Read(fd, buff, 1023);
				buff[amtRead] = '\0';
				
				myConsole->printf("%s", buff);
			}

			delete [] buff;
			fsMan.Close(fd);
		}
		
		else if(input == "clear")
		{
			myConsole->ClearScreen();			
		}
	}
}

// just get and print the characters
void BlankConsole()
{
	VirtualConsole	*myConsole = VirtualConsoleManager::GetInstance().GetCurrentConsole();
	uchar		ch;
	
	myConsole->SetAttribute(VirtualConsole::FG_LIGHT_GREY);
	
	while(1)
	{
		ch = myConsole->GetCharacter();
		
		if(ch == 0x8)
			myConsole->EraseLastCharacter();
		else
			myConsole->printf("%c", ch);
	}		
}



/* SEMAPHORE TEST STUFF
#define	N	4
#define LEFT	((i+N-1)%N)
#define RIGHT	((i+1)%N)
#define THINK	0
#define HUNGRY	1
#define EATING	2

int		state[N];
Semaphore	mutex(1);
vector<Semaphore>	s(N, Semaphore(1));

void test(int i)
{
	if(state[i] == HUNGRY &&
	   state[LEFT] != EATING &&
		  state[RIGHT] != EATING)
	{
		state[i] = EATING;
		s[i].Signal();
	}
}

void take_forks(int i)
{
	mutex.Wait();
	state[i] = HUNGRY;
	test(i);
	mutex.Signal();
	s[i].Wait();
}

void put_forks(int i)
{
	mutex.Wait();
	state[i] = THINK;
	test(LEFT);
	test(RIGHT);
	mutex.Signal();
}


void Phi1(void)
{
	while(1)
	{
		asm("cli");
		printf("PHI 1 IS THINKING\n");
		asm("sti");
		
		take_forks(0);
		
		asm("cli");
		printf("PHI 1 IS EATING\n");
		asm("sti");
		
		put_forks(0);
	}
}

void Phi2(void)
{
	while(1)
	{
		asm("cli");
		printf("PHI 2 IS THINKING\n");
		asm("sti");
		
		take_forks(1);
		
		asm("cli");
		printf("PHI 2 IS EATING\n");
		asm("sti");
		
		put_forks(1);
	}
}

void Phi3(void)
{
	while(1)
	{
		asm("cli");
		printf("PHI 3 IS THINKING\n");
		asm("sti");
		
		take_forks(2);
		
		asm("cli");
		printf("PHI 3 IS EATING\n");
		asm("sti");
		
		put_forks(2);
	}
}

void Phi4(void)
{
	while(1)
	{
		asm("cli");
		printf("PHI 4 IS THINKING\n");
		asm("sti");
		
		take_forks(3);
		
		asm("cli");
		printf("PHI 4 IS EATING\n");
		asm("sti");
		
		put_forks(3);
	}
}
*/
