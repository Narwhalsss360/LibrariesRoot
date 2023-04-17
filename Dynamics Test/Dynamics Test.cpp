#include <iostream>
#include "DynamicArray.h"

template <typename T>
void coutArray(DynamicArray<T>& carr)
{
    for (size_t i = 0; i < carr.getSize(); i++) std::cout << carr[i] << '\n';
}

int main()
{
    DynamicArray<int> arr = DynamicArray<int>(5, 8 * sizeof(int));
    arr[0] = 0;
    arr[1] = 1;
    arr[2] = 2;
    arr[3] = 3;
    arr[4] = 4;
    coutArray(arr);
    arr.erase();
}