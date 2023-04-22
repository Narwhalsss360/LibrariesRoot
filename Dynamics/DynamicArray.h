#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include <memory>
#include <stdint.h>
#endif

#ifdef DynamicArray_EXCEPT
#include <exception>

class DynamicArrayException :: std::exception
{
};
#endif

template <typename T>
class DynamicArray
{
    uint32_t count; //How many T's are instantiated;
	uint32_t size; //Current Allocation of memory for this many T's
	const uint32_t preAllocated; //Always have this amount of memory available for this many T's
	T* array;
public:
	DynamicArray(uint32_t intialSize, uint32_t preAllocation = 0);
	bool resize(uint32_t newSize);
    bool append(T value);
    bool append(T& value);
    uint32_t getSize();
	T* get(uint32_t index);
    T& operator[](uint32_t index);
    void erase();
	~DynamicArray();
};

template <typename T>
DynamicArray<T>::DynamicArray(uint32_t initalSize, uint32_t preAllocation)
    : count(0), size(initalSize), preAllocated(preAllocation), array(nullptr)
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
        if (array == nullptr) goto ErrorOccured;
        size = newSize;
        return true;
    }

    T* temp = new T[(newSize >= size ? size : newSize)];
    memcpy(temp, array, sizeof(T) * (newSize >= size ? size : newSize)); //newSize

    delete[] array;
    array = new T[newSize];

    if (array == nullptr) goto ErrorOccured;

    memcpy(array, temp, sizeof(T) * (newSize >= size ? size : newSize));
    delete[] temp;
    return true;
ErrorOccured:
    #ifdef DynamicArray_EXCEPT
    throw DynamicArrayException();
    #endif
    return false;

}

template <typename T>
bool DynamicArray<T>::append(T value)
{
    return append(value);
}

template <typename T>
bool DynamicArray<T>::append(T& value)
{

}

template <typename T>
uint32_t DynamicArray<T>::getSize()
{
    return size;
}

template <typename T>
T* DynamicArray<T>::get(uint32_t index)
{
#if DynamicArray_EXCEPT
    if (index >= size) throw DynamicArrayException();
    return &array[index];
#else
    return (index < size) ? &array[index] : nullptr;
#endif
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