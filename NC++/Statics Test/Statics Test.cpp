#include <iostream>
#include <string>
#include "StaticArray.h"
#include "StaticArrayIterators.h"
#include <vector>

using std::cin;
using std::cout;
using std::endl;

using std::string;

template <typename T>
void PrintElement(size_t index, const T& item)
{
	cout << item << ' ';
}

template <typename T, size_t Size>
void PrintElementsFormatted(StaticArray<T, Size>& staticArray)
{
	cout << '[';
	for (auto& item : IterateArray(staticArray))
	{
		cout << item;
		if (&item != &staticArray.Last()) cout << ", ";
	}
	cout << ']' << endl;
}

int main()
{
	auto staticArray = StaticArray<int, 5>();

	std::vector<int> nums = { 0, 1, 2 };

	PrintElementsFormatted(staticArray);

	cin.get();
}