#include <iostream>
#include "DynamicArray.h"

template <typename T>
void coutArray(DynamicArray<T>& carr)
{
    for (size_t i = 0; i < carr.getSize(); i++) std::cout << carr[i] << '\n';
}

int main()
{
    DynamicArray<int> arr = DynamicArray<int>();
}