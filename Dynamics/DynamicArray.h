#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include <memory>
#include <stdint.h>
#endif

template <typename T>
class DynamicArray
{
	uint32_t size;
	const uint32_t preAllocated;
	T* array;
public:
	DynamicArray(uint32_t intialSize, uint32_t preAllocation = 0);
	bool resize(uint32_t newSize);
    bool append(T value);
    uint32_t getSize();
	T* get(uint32_t index);
    T& operator[](uint32_t index);
    void erase();
	~DynamicArray();
};

template <typename T>
DynamicArray<T>::DynamicArray(uint32_t initalSize, uint32_t preAllocation)
    : size(initalSize), preAllocated(preAllocation), array(nullptr)
{
    if (preAllocated > 0) array = new T[preAllocated];
}

template <typename T>
bool DynamicArray<T>::resize(uint32_t newSize)
{
    if (newSize < preAllocated)
    {
        size = newSize;
        return true;
    };

    if (newSize == 0)
    {
        delete[] array;
        size = newSize;
        return true;
    }

    if (size == 0)
    {
        array = new T[newSize];
        size = newSize;
        return (array != nullptr);
    }

    T* temp = new T[(newSize >= size ? size : newSize)];
    memcpy(temp, array, sizeof(T) * (newSize >= size ? size : newSize)); //newSize

    delete[] array;
    array = new T[newSize];

    memcpy(array, temp, sizeof(T) * (newSize >= size ? size : newSize));
    delete[] temp;
    return (array != nullptr);
}

template <typename T>
uint32_t DynamicArray<T>::getSize()
{
    return size;
}

template <typename T>
T* DynamicArray<T>::get(uint32_t index)
{
    return (index < size) ? &array[index] : nullptr;
}

template <typename T>
T& DynamicArray<T>::operator[](uint32_t index)
{
    return *get(index);
}

template <typename T>
void DynamicArray<T>::erase()
{
    if (preAllocated == 0)
    {
        delete[] array;
        return;
    }

    resize(0);
    memset(array, 0, sizeof(T) * preAllocated);
}

template <typename T>
DynamicArray<T>::~DynamicArray()
{
    if (preAllocated || size > 0)
    {
        delete[] array;
    }
}