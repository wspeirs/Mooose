#include <iostream>

using std::cout;
using std::endl;

// protos for the tests
void ListTest();
void VectorTest();
void BitVectorTest();
void MapTest();
void StringTest();
void TreeTest();


int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

/*	cout << "*** LIST TEST ***" << endl;
	ListTest();
	cout << endl;

	cout << "*** VECTOR TEST ***" << endl;
	VectorTest();
	cout << endl;

	cout << "*** BIT VECTOR TEST ***" << endl;
	BitVectorTest();
	cout << endl;
	cout << "*** MAP TEST ***" << endl;
	MapTest();
	cout << endl;

	cout << "*** STRING TEST ***" << endl;
	StringTest();
	cout << endl;
*/

	cout << "*** TREE TEST ***" << endl;
	TreeTest();
	cout << endl;

	return(0);
}


