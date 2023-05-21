#include <iostream>
#include <string>
#include "StaticArray.h"

#define stringify_identifier(ident) #ident

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::to_string;

int main()
{
	StaticArray<int, 5> staticIntegerArray(5);

	for (Enumeration<int> collectionItem : Enumerate<int>(staticIntegerArray))
		*collectionItem = (int)floor(pow(2, collectionItem.index));

	for (auto collectionItem : Enumerate<int>(staticIntegerArray))
		cout << '[' << collectionItem.index << "]:" << *collectionItem << ' ';
	cout << '\n';

	cout << "[ ";
	for (auto& item : staticIntegerArray)
		cout << item << ' ';
	cout << "]\n";

	cout << stringify_identifier(staticIntegerArray) << ": " << staticIntegerArray.ToString("", [](const int& elem) -> string { return to_string(elem); }) << endl;
}