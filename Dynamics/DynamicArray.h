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
    enum CODES
    {

    } code;

    DynamicArrayException()
        : std::exception()
    {
    }

    DynamicArrayException(CODES code)
        : std::exception(), code(code)
    {
    }
};
#else
#define CHECK_RESULT(r) (&r == nullptr) ? false : true
#endif

template <typename T>
class DynamicArray
{
    uint32_t count; //How many T's are instantiated;
	uint32_t size; //Current Allocation of memory for this many T's
	const uint32_t preAllocated; //Always have this amount of memory available for this many T's
	T* array;
public:
	DynamicArray(uint32_t preAllocation = 0);
    DynamicArray(T* array, uint32_t arraySize);
    bool resize(uint32_t newSize);

    bool append(T value);
    bool append(T& value);
    bool append(DynamicArray<T>& other);
    bool append(T* array, uint32_t arraySize);
    bool push(T value);
    bool push (T& value);
    bool push(DynamicArray<T>& other);
    bool push(T* array, uint32_t arraySize);
    T pop();

    bool insert(T value, uint32_t index);
    bool insert(T& value, uint32_t index);
    bool insert(DynamicArray<T>& other, uint32_t index);
    bool insert(T* array, uint32_t arraySize, uint32_t index);

    bool prepend(T value);
    bool prepend(T& value);
    bool prepend(DynamicArray<T>& other);
    bool prepend(T* array, uint32_t arraySize);
    bool unshift(T value);
    bool unshift(T& value);
    bool unshift(DynamicArray<T>& other);
    bool unshift(T* array, uint32_t arraySize);
    T shift();

    T replace(T value, uint32_t index);
    T replace(T& value, uint32_t index);

    bool join(DynamicArray<T>& other);
    bool join(T* array, uint32_t arraySize);

    DynamicArray<T> slice(uint32_t index) const;
    DynamicArray<T> slice(uint32_t index, uint32_t count) const;

    bool remove(uint32_t index);

    void forEach(void (*f)(uint32_t, T&));
    void forEach(void (*f)(uint32_t, const T&)) const;

    const int64_t indexOf(T comparator) const;
    const int64_t lastIndexOf(T comparator) const;
    const int64_t find(bool (*comparator)(const T&)) const;
    const int64_t lastFind(bool (*comparator)(const T&)) const;
    const uint32_t countOf(T comparator) const;
    const uint32_t countOf(bool (*comparator)(const T&)) const;

    const uint32_t getSize() const;
    const uint32_t getCount() const;

	const T* get(uint32_t index);
    const T* const get(uint32_t index) const;
    T& operator[](uint32_t index);
    const T& operator[](uint32_t index) const;

    void erase();
	~DynamicArray();
};

template <typename T>
DynamicArray<T>::DynamicArray(uint32_t preAllocation)
    : count(0), size(preAllocation), preAllocated(preAllocation), array(nullptr)
{
    if (preAllocated > 0) array = new T[preAllocated];
}

template <typename T>
DynamicArray<T>::DynamicArray(T* array, uint32_t arraySize)
    : count(arraySize), size(arraySize), preAllocated(0), array(nullptr)
{
    if (arraySize)
    {
retry:
        array = new T[arraySize];
        if (array == nullptr)
#ifdef DynamicArray_EXCEPT
            throw DynamicArrayException();
#else
            goto retry;
#endif

        for (uint32_t i = 0; i < arraySize; i++) this->array[i] = array[i];
    }
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

    T* newArray = new T[newSize];
    if (newArray == nullptr) goto ErrorOccured;
    for (uint32_t i = 0; i < count; i++) newArray[i] = array[i];
    delete[] array;
    array = newArray;
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
    if (count == size) if (!resize(size + 1)) return false;
    array[count] = value;
    count++;
    return true;
}

template <typename T>
bool DynamicArray<T>::append(DynamicArray<T>& other)
{
    return join(other);
}

template <typename T>
bool DynamicArray<T>::append(T* array, uint32_t arraySize)
{
    return join(array, arraySize);
}

template <typename T>
bool DynamicArray<T>::push(T value)
{
    return append(value);
}

template <typename T>
bool DynamicArray<T>::push(T& value)
{
    return append(value);
}

template <typename T>
bool DynamicArray<T>::push(DynamicArray<T>& other)
{
    return join(other);
}

template <typename T>
bool DynamicArray<T>::push(T* array, uint32_t arraySize)
{
    return join(array, arraySize);
}

template <typename T>
T DynamicArray<T>::pop()
{
    if (count == 0)
#ifdef DynamicArray_EXCEPT
    throw DynamicArrayException();
#else
    return *(T*)nullptr;
#endif
    T popped = remove(count - 1);
    return popped;
}

template <typename T>
bool DynamicArray<T>::insert(T value, uint32_t index)
{
    return insert(value, index);
}

template <typename T>
bool DynamicArray<T>::insert(T& value, uint32_t index)
{
    if (count == size) if (!resize(size + 1)) return false;
    for (uint32_t i = count - 1; i > index; i--) array[i + 1] = array[i];
    array[index] = value;
    count++;
    return true;
}

template <typename T>
bool DynamicArray<T>::insert(DynamicArray<T>& other, uint32_t index)
{
    return insert(other.array, index);
}

template <typename T>
bool DynamicArray<T>::insert(T* array, uint32_t arraySize, uint32_t index)
{
    if (arraySize == 0) return true;
    if (count + arraySize > size) if (!resize(count + arraySize)) return false;
    for (uint32_t i = count - 1; i > index; i--) this->array[i + arraySize] = this->array[i];
    for (uint32_t i = 0; i < arraySize; i++) this->array[i + index] = array[i];
    count += arraySize;
    return true;
}

template <typename T>
bool DynamicArray<T>::prepend(T value)
{
    return prepend(value);
}

template <typename T>
bool DynamicArray<T>::prepend(T& value)
{
    if (count == size) if (!resize(size + 1)) return false;
    for (uint32_t i = count - 1; i > 0; i--;) array[i + 1] = array[i];
    array[0] = value;
    count++;
    return true;
}

template <typename T>
bool prepend(DynamicArray<T>& other)
{
    return prepend(other.array, other.count);
}

template <typename T>
bool prepend(T* array, uint32_t arraySize)
{
    if (count + arraySize > size) if (!resize(count + arraySize)) return false;
    for (uint32_t i = count - 1; i >= arraySize; i--;) this->array[i + arraySize] = this->array[i];
    for (uint32_t i = 0; i < arraySize; i++) this->array[i] = array[i];
    count += count;
    return true;
}

template <typename T>
bool DynamicArray<T>::unshift(T value)
{
    return prepend(value);
}

template <typename T>
bool DynamicArray<T>::unshift(T value)
{
    return prepend(value);
}

template <typename T>
bool DynamicArray<T>::unshift(DynamicArray<T>& other)
{
    return prepend(other);
}

template <typename T>
bool DynamicArray<T>::unshift(T* array, uint32_t arraySize)
{
    return prepend(array, arraySize);
}

template <typename T>
T DynamicArray<T>::shift()
{
    if (count == 0)
#ifdef DynamicArray_EXCEPT
    throw DynamicArrayException();
#else
    return *(T*)nullptr;
#endif
    T shifted = remove(0);
    return shifted;
}

template <typename T>
T DynamicArray<T>::replace(T value, uint32_t index)
{
    return replace(value, index);
}

template <typename T>
T DynamicArray<T>::replace(T& value, uint32_t index)
{
    if (index >= count)
#ifdef DynamicArray_EXCEPT
    throw DynamicArrayException();
#else
    return *(T*)nullptr;
#endif
    T copy = array[index];
    array[index] = value;
    return copy;
}

template <typename T>
bool DynamicArray<T>::join(DynamicArray<T>& other)
{
    return join(other.array, other.count);
}

template <typename T>
bool DynamicArray<T>::join(T* array, uint32_t arraySize)
{
    if (arraySize == 0) return true;
    if (count + arraySize > size) if (!resize(count + arraySize)) return false;
    for (uint32_t i = 0; i < arraySize; i++) if (!append(array[i])) return false;
    return true;
}

template <typename T>
DynamicArray<T> DynamicArray<T>::slice(uint32_t index) const
{
    return slice(index, count - index);
}

template <typename T>
DynamicArray<T> DynamicArray<T>::slice(uint32_t index, uint32_t count) const
{
    if (count + index > count) count = this->count - index;
    return DynamicArray<T>(&array[index], count);
}

template <typename T>
bool remove(uint32_t index)
{
    for (uint32_t i = index; i < count - 1; i++) array[i] = array[i + 1];
    return true;
}

template <typename T>
void DynamicArray<T>::forEach(void (*f)(uint32_t, T&))
{
    for (uint32_t i = 0; i < count; i++) f(i, array[i]);
}

template <typename T>
void DynamicArray<T>::forEach(void (*f)(uint32_t, const T&)) const
{
    for (uint32_t i = 0; i < count; i++) f(i, array[i]);
}

template <typename T>
const int64_t DynamicArray<T>::indexOf(T comparator) const
{
    for (uint32_t i = 0; i < count; i++) if (array[i] == comparator) return i;
    return -1;    
}

template <typename T>
const int64_t DynamicArray<T>::lastIndexOf(T comparator) const
{
    for (uint32_t i = count - 1; i >= 0; i--) if (array[i] == comparator) return i;
    return -1;    
}

template <typename T>
const int64_t DynamicArray<T>::find(bool (*comparator)(const T&)) const
{
    for (uint32_t i = 0; i < count; i++) if (comparator(array[i])) return i;
    return -1;
}

template <typename T>
const int64_t DynamicArray<T>::lastFind(bool (*comparator)(const T&)) const
{
    for (uint32_t i = count - 1; i >= 0; i--) if (comparator(array[i])) return i;
    return -1;
}

template <typename T>
const uint32_t DynamicArray<T>::countOf(T comparator) const
{
    uint32_t counts = 0;
    for (uint32_t i = 0; i < count; i++) if (array[i] == comparator) count++;
    return count;
}

template <typename T>
const uint32_t DynamicArray<T>::countOf(bool (*comparator)(const T&)) const
{
    uint32_t counts = 0;
    for (uint32_t i = 0; i < count; i++) if (comparator(array[i])) count++;
    return count;
}

template <typename T>
const uint32_t DynamicArray<T>::getSize() const
{
    return size;
}

template <typename T>
const uint32_t DynamicArray<T>::getCount() const
{
    return count;
}

template <typename T>
const T* DynamicArray<T>::get(uint32_t index)
{
#ifdef DynamicArray_EXCEPT
    if (index >= count) throw DynamicArrayException();
    return &array[index];
#else
    return (index < count) ? &array[index] : nullptr;
#endif
}

template <typename T>
const T* const DynamicArray<T>::get(uint32_t index) const
{
#ifdef DynamicArray_EXCEPT
    if (index >= count) throw DynamicArrayException();
    return &array[index];
#else
    return (index < count) ? &array[index] : nullptr;
#endif
}

template <typename T>
T& DynamicArray<T>::operator[](uint32_t index)
{
    return *get(index);
}

template <typename T>
const T& DynamicArray<T>::operator[](uint32_t index) const
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