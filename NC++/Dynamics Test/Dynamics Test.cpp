#include <iostream>
#include "DynamicArray.h"

#define showArrayWithName(arr) std::cout << #arr << ": " << arr
#define showArrayWithIndex(arr) F_showArrayWithIndex(#arr, arr)

std::string intToString(int& i)
{
	return std::to_string(i);
}

void add10ToEach(uint32_t index, int& value)
{
	value += 10;
}

void binarize(uint32_t index, int& value)
{
	value = pow(2, index);
}

template <typename T>
void F_showArrayWithIndex(std::string name, DynamicArray<T> array)
{
	std::cout << name << "=>";
	uint32_t idx = 0;
	for (auto& item : array)
	{
		std::cout << '[' << idx << "]:" << item;
		if (idx != array.getCount() - 1) std::cout << ", ";
		++idx;
	}
}

int main()
{
    DynamicArray<int> inOrderArray = DynamicArray<int>();
	const int testLength = 12;

	for (int i = 0; i < testLength; i++) inOrderArray += i;

	int cArray[3] = { 0, 1, 2 };
	DynamicArray<int> fromCArray = DynamicArray<int>(cArray, 3);

	showArrayWithName(inOrderArray) << std::endl;

	inOrderArray.prepend(fromCArray);
	showArrayWithName(inOrderArray) << '\n';

	fromCArray.insert(8, 1);
	showArrayWithName(fromCArray) << "\n";

	int evensCArray[4] = { 0, 2, 4, 6 };

	DynamicArray<int> evens = DynamicArray<int>(evensCArray, 4);

	evens.insert(fromCArray, 4, 2);

	showArrayWithName(evens) << "\n";

	evens.remove(3);

	showArrayWithName(evens) << "\n";

	for (uint32_t i = 1; i < evens.getCount() - 1; i++) while (evens[i] - 2 != evens[i - 1]) evens.remove(i);

	showArrayWithName(evens) << "\n";

	evens.forEach(add10ToEach);

	showArrayWithName(evens) << "\n";

	showArrayWithIndex(evens);
	std::cout << '\n';

	evens ^ binarize;
	std::cout << "Binarizing\n";
	showArrayWithIndex(evens);

	std::cin.get();
}