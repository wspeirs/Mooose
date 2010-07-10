#include <iostream>
#include <vector>

using std::cout;
using std::endl;

typedef std::vector<bool>	bit_vector;
typedef bit_vector::iterator	bit_vector_it;

#define UNIT_TEST

#include "vector.h"
#include "algorithms.h"

typedef k_std::vector<bool>	k_bit_vector;
typedef k_bit_vector::iterator	k_bit_vector_it;

#define SIZE	512

void BitVectorTest()
{
	k_bit_vector	kVec1, kVec3(SIZE, false);
	bit_vector	list1, list3(SIZE, false);

	cout << "K Bit Vector 3: ";
	for(k_bit_vector_it it = kVec3.begin(), end = kVec3.end(); it != end; ++it)
		cout << *it << ",";
	cout << endl;

	cout << "  Bit Vector 3: ";
	for(bit_vector_it it = list3.begin(); it != list3.end(); ++it)
		cout << *it << ",";
	cout << endl << endl;

	uint	count = 0;
	
	for(int i=0; i < SIZE; i++)
	{
		if(!(i%2))
			++count;
		
		kVec1.push_back(!(i%2));
		list1.push_back(!(i%2));
	}

	cout << count << " SHOULD = ";
	
	count = 0;
	
	k_bit_vector_it it;
	
	do
	{
		it = find(kVec1.begin(), kVec1.end(), true);
		++count;
		
		if(it != kVec1.end())
			*it = false;
		
	} while(it != kVec1.end());

	cout << --count << endl;
	
	//
	/// After the above test, the vec1s won't be the same
	//
	
	cout << "K Bit Vector 1: ";
	for(k_bit_vector_it it = kVec1.begin(); it != kVec1.end(); ++it)
		cout << *it << " ";
	cout << endl;

	cout << "  Bit Vector 1: ";
	for(bit_vector_it it = list1.begin(); it != list1.end(); ++it)
		cout << *it << " ";
	cout << endl << endl;

	kVec1.insert(kVec1.end(), kVec3.begin(), kVec3.end());
	list1.insert(list1.end(), list3.begin(), list3.end());

	cout << "K Bit Vector 1: ";
	for(k_bit_vector_it it = kVec1.begin(); it != kVec1.end(); ++it)
		cout << *it << " ";
	cout << endl;

	cout << "  Bit Vector 1: ";
	for(bit_vector_it it = list1.begin(); it != list1.end(); ++it)
		cout << *it << " ";
	cout << endl << endl;

	kVec1.insert(kVec1.end(), SIZE, true);
	list1.insert(list1.end(), SIZE, true);

	cout << "K Bit Vector 1: ";
	for(k_bit_vector_it it = kVec1.begin(); it != kVec1.end(); ++it)
		cout << *it << " ";
	cout << endl;

	cout << "  Bit Vector 1: ";
	for(bit_vector_it it = list1.begin(); it != list1.end(); ++it)
		cout << *it << " ";
	cout << endl << endl;

	cout << "K Bit Vector 3: ";
	for(k_bit_vector_it it = kVec3.begin(); it != kVec3.end(); ++it)
		cout << *it << " ";
	cout << endl;

	cout << "  Bit Vector 3: ";
	for(bit_vector_it it = list3.begin(); it != list3.end(); ++it)
		cout << *it << " ";
	cout << endl << endl;

	cout << "Sizes: " << kVec1.size() << " " <<  kVec3.size() << endl;
	cout << "       " << list1.size() << " " <<  list3.size() << endl << endl;

	cout << "K Bit Vector 1: ";
	for(uint i = 0; i <  kVec1.size(); ++i)
		cout << kVec1[i] << " "; 
	cout << endl;

	cout << "  Bit Vector 1: ";
	for(uint i = 0; i < list1.size(); ++i)
		cout << list1[i] << " "; 
	cout << endl << endl;

	// Start erasing things
	kVec1.erase(++kVec1.begin(), kVec1.end());
	list1.erase(++list1.begin(), list1.end());

	cout << "K Bit Vector 1: ";
	for(uint i = 0; i <  kVec1.size(); ++i)
		cout << kVec1[i] << " "; 
	cout << endl;

	cout << "  Bit Vector 1: ";
	for(uint i = 0; i < list1.size(); ++i)
		cout << list1[i] << " "; 
	cout << endl << endl;

	for(int i=0; i < SIZE; ++i)
	{
		kVec3.pop_back();
		list3.pop_back();
	}

	cout << "K Bit Vector 1: ";
	for(uint i = 0; i <  kVec1.size(); ++i)
		cout << kVec1[i] << " "; 
	cout << endl;

	cout << "  Bit Vector 1: ";
	for(uint i = 0; i < list1.size(); ++i)
		cout << list1[i] << " "; 
	cout << endl << endl;

	cout << "K Bit Vector 3: ";
	for(uint i = 0; i <  kVec3.size(); ++i)
		cout << kVec3[i] << " "; 
	cout << endl;

	cout << "  Bit Vector 3: ";
	for(uint i = 0; i < list3.size(); ++i)
		cout << list3[i] << " "; 
	cout << endl << endl;

	cout << "Sizes: " << kVec1.size() << kVec3.size() << endl;
	cout << "       " << list1.size() << list3.size() << endl;
}

