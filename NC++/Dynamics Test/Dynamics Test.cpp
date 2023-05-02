#include <iostream>
#include "DynamicArray.h"

std::string intToString(int& i)
{
	return std::to_string(i);
}

int main()
{
    DynamicArray<int> inOrderArray = DynamicArray<int>();
	const int testLength = 12;

	for (int i = 0; i < testLength; i++)
	{
		inOrderArray.prepend(i);
	}

	std::cout << inOrderArray;
}