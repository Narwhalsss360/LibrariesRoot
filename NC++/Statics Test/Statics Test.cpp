#include <iostream>

#include "Iterable.h"
#include "StaticArray.h"

using std::cin;
using std::cout;
using std::endl;

int main()
{
	StaticArrayInit(int, staticArray, { 1, 2, 3 });
	StaticArray<int, 3> staticArray2({ 1, 2 });
	int col[] = { 0, 1, 2, 4 };

	sizeof(int);
	sizeof(int*);
	sizeof(int*);

	static_for(int, col)
	{
		std::cout << col[index_col];
	}
}