#include <iostream>
#include <vector>

using std::cout;
using std::endl;

typedef std::vector<int>	vector_int;
typedef vector_int::iterator	vector_it;

#include "vector.h"

typedef k_std::vector<int>	k_vector_int;
typedef k_vector_int::iterator	k_vector_it;

#define SIZE	1

void VectorTest()
{
	k_vector_int	kVec1, kVec3(SIZE, 6);
	vector_int	list1, list3(SIZE, 6);

	for(int i=0; i < SIZE; i++)
	{
		kVec1.push_back(i);
		list1.push_back(i);
	}

	kVec1.insert(kVec1.end(), kVec3.begin(), kVec3.end());
	list1.insert(list1.end(), list3.begin(), list3.end());

	cout << "K Vector 1: ";
	for(k_vector_it it = kVec1.begin(); it != kVec1.end(); ++it)
		cout << *it << " ";
	cout << endl;

	cout << "  Vector 1: ";
	for(vector_it it = list1.begin(); it != list1.end(); ++it)
		cout << *it << " ";
	cout << endl << endl;

	kVec1.insert(kVec1.end(), SIZE, 7);
	list1.insert(list1.end(), SIZE, 7);

	cout << "K Vector 1: ";
	for(k_vector_it it = kVec1.begin(); it != kVec1.end(); ++it)
		cout << *it << " ";
	cout << endl;

	cout << "  Vector 1: ";
	for(vector_it it = list1.begin(); it != list1.end(); ++it)
		cout << *it << " ";
	cout << endl << endl;

	kVec1.sort();
	sort(list1.begin(), list1.end());

	cout << "K Vector 1: ";
	for(k_vector_it it = kVec1.begin(); it != kVec1.end(); ++it)
		cout << *it << " ";
	cout << endl;

	cout << "  Vector 1: ";
	for(vector_it it = list1.begin(); it != list1.end(); ++it)
		cout << *it << " ";
	cout << endl << endl;

	cout << "K Vector 3: ";
	for(k_vector_it it = kVec3.begin(); it != kVec3.end(); ++it)
		cout << *it << " ";
	cout << endl;

	cout << "  Vector 3: ";
	for(vector_it it = list3.begin(); it != list3.end(); ++it)
		cout << *it << " ";
	cout << endl << endl;

	cout << "Sizes: " << kVec1.size() << kVec3.size() << endl;
	cout << "       " << list1.size() << list3.size() << endl;

	// Start erasing things
	kVec1.erase(++kVec1.begin(), kVec1.end());
	list1.erase(++list1.begin(), list1.end());

	for(int i=0; i < SIZE; ++i)
	{
		kVec3.pop_back();
		list3.pop_back();
	}

	cout << "K Vector 1: ";
	for(uint i = 0; i <  kVec1.size(); ++i)
		cout << kVec1[i] << " "; 
	cout << endl;

	cout << "  Vector 1: ";
	for(uint i = 0; i < list1.size(); ++i)
		cout << list1[i] << " "; 
	cout << endl << endl;

	cout << "K Vector 3: ";
	for(uint i = 0; i <  kVec3.size(); ++i)
		cout << kVec3[i] << " "; 
	cout << endl;

	cout << "  Vector 3: ";
	for(uint i = 0; i < list3.size(); ++i)
		cout << list3[i] << " "; 
	cout << endl << endl;

	cout << "Sizes: " << kVec1.size() << kVec3.size() << endl;
	cout << "       " << list1.size() << list3.size() << endl;
}

