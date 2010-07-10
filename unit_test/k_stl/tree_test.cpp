#include "rb_tree.h"
#include <vector>
#include <algorithm>

using k_std::rb_tree;
using std::vector;
using std::generate;
using std::random_shuffle;

#define NUM 4000

struct gen
{
	gen(int x) : a(x) { }
	int operator()()
	{ return a++; }

	int a;
};

void TreeTest()
{
	rb_tree<int>	rb;
	vector<int>	input(NUM, 0);

	generate(input.begin(), input.end(), gen(0));

	random_shuffle(input.begin(), input.end());

	//for(int i=0; i < NUM; ++i)
	for(vector<int>::iterator it = input.begin(); it != input.end(); ++it)
	{
//		printf("INSERT: %d\n", i);
		rb.insert(*it);
	}

	printf("AFTER INSERT\n");
//	rb.walk_tree();
	printf("BALANCED: %s\n", rb.is_balanced() ? "YES" : "NO");

	for(int i=0; i < NUM; ++i)
	{
//		printf("ERASE: %d\n", i);
		rb.erase(i);
	}

	printf("AFTER ERASE\n");
	rb.walk_tree();

}

