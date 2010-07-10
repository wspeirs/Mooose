#include "string.h"

#include <stdio.h>

using k_std::string;

void StringTest()
{
	string	str1("test"), str2(5, 'a');
	
	printf("STR1 LEN: %d\tCAP: %d\n", str1.buffLength, str1.buffCapacity);
	printf("STR2 LEN: %d\tCAP: %d\n", str2.buffLength, str2.buffCapacity);

	printf("test = %s\n", str1.c_str());	
	printf("aaaa = %s\n", str2.c_str());
	
	string str3 = str2;
	
	printf("STR3: %s\tLEN: %d\tCAP: %d\n", str3.c_str(), str3.buffLength, str3.buffCapacity);
	
	str3.insert(str3.begin(), str1.begin(), str1.end());
	
	printf("STR3: %s\tLEN: %d\tCAP: %d\n", str3.c_str(), str3.buffLength, str3.buffCapacity);
	
	for(int i=0; i < 12; ++i)
	{
		str3.push_front('z');
	
		printf("STR3: %s\tLEN: %d\tCAP: %d\n", str3.c_str(), str3.buffLength, str3.buffCapacity);
	}
	
	string	str4;
	
	str4 = "blah";
	
	printf("STR4: %s\tLEN: %d\tCAP: %d\n", str4.c_str(), str4.buffLength, str4.buffCapacity);
	
	str4.erase(str4.begin(), str4.end());
	
	printf("STR4: %s\tLEN: %d\tCAP: %d\n", str4.c_str(), str4.buffLength, str4.buffCapacity);
}

