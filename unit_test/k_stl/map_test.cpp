#include "map.h"
#include "string.h"


using k_std::string;

/*
struct GenPair
{
	GenPair()
	{ srand(0xBEDBEEF); }
	
	std::pair<int, int> operator()(void)
	{
		return std::pair<int, int>(rand()%20, rand()%20);
	}
};

struct k_GenPair
{
	k_GenPair()
	{ srand(0xBEDBEEF); }
	
	k_std::pair<int, int> operator()(void)
	{
		return k_std::pair<int, int>(rand()%20, rand()%20);
	}
};*/

#define	SIZE	0

class Handle
{
public:
	Handle() { ; }
	

private:
	char	blah[20];
};

typedef k_std::map<uchar, Handle*>	k_map_handle;
typedef k_map_handle::iterator		k_map_it;
typedef k_std::pair<uchar, Handle*>	pair_handle;


void MapTest()
{
	Handle		*h = new Handle;
	k_map_handle	test;
	
	for(uchar i=0; i < 32; ++i)
		test.insert(pair_handle(i, h));
	
	printf("SIZE: %d\n", test.size());

	delete h;
}


// void MapTest()
// {
// 	// setup the std map
// 	std::pair<int, int>		tmp(0, 0);
// 	std::list<std::pair<int, int> >	t_list(SIZE, tmp);
// 	map_int				months;
// 	
// 	generate(t_list.begin(), t_list.end(), GenPair());
// 	months.insert(t_list.begin(), t_list.end());
// 	
// 	// setup the k_std map
// 	k_std::pair<int, int>			k_tmp(0, 0);
// 	std::list<k_std::pair<int, int> >	kt_list(SIZE, k_tmp);
// 	k_map_int				k_months;
// 	
// 	generate(kt_list.begin(), kt_list.end(), k_GenPair());
// 	k_months.insert(kt_list.begin(), kt_list.end());
// 	
// 	// print out the lists
// 	for(map_it it = months.begin(); it != months.end(); ++it)
// 		cout << (*it).first << "," << (*it).second << "\t";
// 	cout << endl;
// 	
// 	for(k_map_it it = k_months.begin(); it != k_months.end(); ++it)
// 		cout << (*it).first << "," << (*it).second << "\t";
// 	cout << endl << endl;
// 	
// 	// find a random one
// 	cout << (*months.find((*t_list.begin()).first)).second << endl;
// 	cout << (*k_months.find((*kt_list.begin()).first)).second << endl;
// 	cout << endl;
// 	
// 	// delete them all with erase
//  	months.erase(months.begin());
//   	k_months.erase(k_months.begin());
// 
// 	// print out the lists
// 	for(map_it it = months.begin(); it != months.end(); ++it)
// 		cout << (*it).first << "," << (*it).second << "\t";
// 	cout << endl;
// 	
// 	for(k_map_it it = k_months.begin(); it != k_months.end(); ++it)
// 		cout << (*it).first << "," << (*it).second << "\t";
// 	cout << endl << endl;
// 	
// 	months.erase(months.begin(), months.end());
//  	k_months.erase(k_months.begin(), k_months.end());
// 	
// 	// print out the lists
// 	for(map_it it = months.begin(); it != months.end(); ++it)
// 		cout << (*it).first << "," << (*it).second << "\t";
// 	cout << endl;
// 	
// 	for(k_map_it it = k_months.begin(); it != k_months.end(); ++it)
// 		cout << (*it).first << "," << (*it).second << "\t";
// 	cout << endl << endl;
// 	
// 
// }


