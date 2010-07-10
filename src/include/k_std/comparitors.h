#ifndef _COMPARITORS_H
#define _COMPARITORS_H

namespace k_std
{

// create a less_then comparitor
template<typename T>
struct less_then
{
	bool operator()(const T &x, const T &y)
	{ return(x < y); }
};

// create a less_then comparitor
template<typename T>
struct greater_then
{
	bool operator()(const T &x, const T &y)
	{ return(x > y); }
};

template<typename T>
struct equal
{
	bool operator()(const T &x, const T &y)
	{ return(x == y); }
};

template<typename T>
struct not_equal
{
	bool operator()(const T &x, const T &y)
	{ return(x != y); }
};

}

#endif

