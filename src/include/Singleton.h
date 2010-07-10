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

/** @file Singleton.h
 * The singleton base class.
 */

#ifndef SINGLETON_H
#define SINGLETON_H

#ifdef UNIT_TEST

#include "harness_code.h"

#else


#endif

/** @class Singleton
 * The singleton base class.
 */
template<typename T>
class Singleton
{
public:
	static inline T& GetInstance()
	{
		created = true;
		static T instance;	// create the only static instance
		
		return instance;
	}
	
protected:
	Singleton()
	{
// 		if(!created)
// 			;//PANIC("You attempted to construct a Singleton object...\nThis is wrong, call GetInstance()", false);
	}
	~Singleton() { ; }
	
private:
	Singleton(const Singleton &s);
	Singleton& operator=(const Singleton &s);
	static bool created;
};

template<typename T>
bool Singleton<T>::created = false;

//#include <Debug.h>

#endif // SINGLETON_H


