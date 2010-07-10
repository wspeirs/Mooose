/*
#include <iostream>
#include <list>

typedef std::list<int>		list_int;
typedef list_int::iterator	list_it;

using std::cout;
using std::endl;
*/

#include "list.h"

using k_std::list;

#define SIZE 1

struct Items
{
	uchar	a;
	ulong	b;
};

void fun(list<Items> &theList)
{
	Items	tmp;

	for(int i=0; i < SIZE; ++i)
	{
		tmp.a = 'a';
		tmp.b = 3;
		
		theList.push_back(tmp);
	}
}

void ListTest()
{
	list<Items>	theList;
	
	fun(theList);
}





// void ListTest()
// {
// 	k_list_int	kList1, kList2, kList3(SIZE, 6);
// 	list_int	list1, list2, list3(SIZE, 6);
// 
// 	for(int i=0; i < SIZE; i++)
// 	{
// 		kList1.push_back(i);
// 		list1.push_back(i);
// 
// 		kList2.push_front(i);
// 		list2.push_front(i);
// 	}
// 
// 	kList1.insert(kList1.end(), kList2.begin(), kList2.end());
// 	list1.insert(list1.end(), list2.begin(), list2.end());
// 
// 	cout << "K List 1: ";
// 	for(k_list_it it = kList1.begin(); it != kList1.end(); ++it)
// 		cout << *it << " ";
// 	cout << endl;
// 
// 	cout << "  List 1: ";
// 	for(list_it it = list1.begin(); it != list1.end(); ++it)
// 		cout << *it << " ";
// 	cout << endl << endl;
// 
// 	kList1.insert(kList1.begin(), kList3.begin(), kList3.end());
// 	list1.insert(list1.begin(), list3.begin(), list3.end());
// 	
// 	kList1.insert(kList1.end(), SIZE, 7);
// 	list1.insert(list1.end(), SIZE, 7);
// 
// 	cout << "K List 1: ";
// 	for(k_list_it it = kList1.begin(); it != kList1.end(); ++it)
// 		cout << *it << " ";
// 	cout << endl;
// 
// 	cout << "  List 1: ";
// 	for(list_it it = list1.begin(); it != list1.end(); ++it)
// 		cout << *it << " ";
// 	cout << endl << endl;
// 
// 	kList1.sort();
// 	list1.sort();
// 
// 	cout << "K List 1: ";
// 	for(k_list_it it = kList1.begin(); it != kList1.end(); ++it)
// 		cout << *it << " ";
// 	cout << endl;
// 
// 	cout << "  List 1: ";
// 	for(list_it it = list1.begin(); it != list1.end(); ++it)
// 		cout << *it << " ";
// 	cout << endl << endl;
// 
// 	cout << "K List 2: ";
// 	for(k_list_it it = kList2.begin(); it != kList2.end(); ++it)
// 		cout << *it << " ";
// 	cout << endl;
// 
// 	cout << "  List 2: ";
// 	for(list_it it = list2.begin(); it != list2.end(); ++it)
// 		cout << *it << " ";
// 	cout << endl << endl;
// 
// 	cout << "K List 3: ";
// 	for(k_list_it it = kList3.begin(); it != kList3.end(); ++it)
// 		cout << *it << " ";
// 	cout << endl;
// 
// 	cout << "  List 3: ";
// 	for(list_it it = list3.begin(); it != list3.end(); ++it)
// 		cout << *it << " ";
// 	cout << endl << endl;
// 
// 	cout << "Sizes: " << kList1.size() << kList2.size() << kList3.size() << endl;
// 	cout << "       " << list1.size() << list2.size() << list3.size() << endl;
// 
// 	// Start erasing things
// 	kList1.erase(kList1.begin(), --kList1.end());
// 	list1.erase(list1.begin(), --list1.end());
// 
// 	for(int i=0; i < SIZE; ++i)
// 	{
// 		kList2.pop_front();
// 		kList3.pop_back();
// 		list2.pop_front();
// 		list3.pop_back();
// 	}
// 
// 	cout << "K List 1: ";
// 	for(k_list_it it = kList1.begin(); it != kList1.end(); ++it)
// 		cout << *it << " "; 
// 	cout << endl;
// 
// 	cout << "  List 1: ";
// 	for(list_it it = list1.begin(); it != list1.end(); ++it)
// 		cout << *it << " "; 
// 	cout << endl << endl;
// 
// 	cout << "K List 2: ";
// 	for(k_list_it it = kList2.begin(); it != kList2.end(); ++it)
// 		cout << *it << " "; 
// 	cout << endl;
// 
// 	cout << "  List 2: ";
// 	for(list_it it = list2.begin(); it != list2.end(); ++it)
// 		cout << *it << " "; 
// 	cout << endl << endl;
// 
// 	cout << "K List 3: ";
// 	for(k_list_it it = kList3.begin(); it != kList3.end(); ++it)
// 		cout << *it << " "; 
// 	cout << endl;
// 
// 	cout << "  List 3: ";
// 	for(list_it it = list3.begin(); it != list3.end(); ++it)
// 		cout << *it << " "; 
// 	cout << endl << endl;
// 
// 	cout << "Sizes: " << kList1.size() << kList2.size() << kList3.size() << endl;
// 	cout << "       " << list1.size() << list2.size() << list3.size() << endl;
// }

