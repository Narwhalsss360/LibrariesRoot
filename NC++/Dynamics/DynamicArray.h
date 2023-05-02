#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#define str_type String
#define SKIP_STREAM_OVERLOAD
#else
#include <memory>
#include <stdint.h>
#include <string>
#include <sstream>
#define str_type std::string
#define USE_SS
#if !defined(SKIP_STREAM_OVERLOAD)
#include <iostream>
#endif
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

#define GET_rfor_index() __begin.getIndex()

template <typename T>
class DynamicArray;

template <typename T>
class DynamicArrayRange
{
    friend class DynamicArray<T>;
    uint32_t index;
    DynamicArray<T>* array;
public:
    DynamicArrayRange(uint32_t index, DynamicArray<T>* array);
    bool operator !=(DynamicArrayRange<T>);
    T& operator*();
    void operator++();
    uint32_t getIndex();
    ~DynamicArrayRange();
};

template <typename T>
class DynamicArray
{
    uint32_t count; //How many T's are instantiated;
	uint32_t size; //Current Allocation of memory for this many T's
	const uint32_t preAllocated; //Always have this amount of memory available for this many T's
	T* array;

public:
    str_type(*TtoStringFunction) (T&);

	DynamicArray(uint32_t preAllocation = 0);
    DynamicArray(T* array, uint32_t arraySize);
    DynamicArray(const DynamicArray<T>& other);
    bool resize(uint32_t newSize);

    bool append(T value);
    bool append(DynamicArray<T> other);
    bool append(T* array, uint32_t arraySize);
    bool push(T value);
    bool push(DynamicArray<T> other);
    bool push(T* array, uint32_t arraySize);
    T pop();

    bool insert(T value, uint32_t index);
    bool insert(DynamicArray<T>& other, uint32_t index);
    bool insert(T* array, uint32_t arraySize, uint32_t index);

    bool prepend(T value);
    bool prepend(DynamicArray<T> other);
    bool prepend(T* array, uint32_t arraySize);
    bool unshift(T value);
    bool unshift(DynamicArray<T> other);
    bool unshift(T* array, uint32_t arraySize);
    T shift();

    bool swap(uint32_t indexA, uint32_t indexB);

    T replace(T value, uint32_t index);

    bool join(DynamicArray<T>& other);
    bool join(T* array, uint32_t arraySize);

    DynamicArray<T> slice(uint32_t index) const;
    DynamicArray<T> slice(uint32_t index, uint32_t count) const;

    bool set(T* array, uint32_t arraySize);
    void setPointer(T* array, uint32_t allocation, uint32_t count);

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

	T* const get(uint32_t index);
    const T* const get(uint32_t index) const;

    str_type toString(str_type start = "[", str_type del = ", ", str_type end = "]", str_type (*TtoString)(const T&) = nullptr) const;

    DynamicArray<T>& operator=(const DynamicArray<T>& other);

    T& operator[](uint32_t index);
    const T& operator[](uint32_t index) const;

    DynamicArray<T>& operator+=(T value);
    DynamicArray<T>& operator+=(DynamicArray<T> other);
    DynamicArray<T>& operator+=(DynamicArray<T>& other);

    DynamicArray<T>& operator+(const DynamicArray<T> other) const;
    DynamicArray<T>& operator+(const DynamicArray<T>& other) const;

    bool operator==(DynamicArray<T>& other) const;
    bool operator!=(DynamicArray<T>& other) const;
    bool operator>(DynamicArray<T>& other) const;
    bool operator<(DynamicArray<T>& other) const;
    bool operator>=(DynamicArray<T>& other) const;
    bool operator<=(DynamicArray<T>& other) const;
    operator T*() const;
    operator str_type() const;

    DynamicArrayRange<T> begin();
    DynamicArrayRange<T> end();

    void erase();
	~DynamicArray();
};

#if !defined(SKIP_STREAM_OVERLOAD)
template <typename T>
std::ostream& operator<<(std::ostream& out, const DynamicArray<T>& dynamicArray)
{
    out << dynamicArray.toString();
    return out;
}
#endif

#pragma region DynamicArray
template <typename T>
DynamicArray<T>::DynamicArray(uint32_t preAllocation)
    : count(0), size(preAllocation), preAllocated(preAllocation), array(nullptr), TtoStringFunction(nullptr)
{
    if (preAllocated > 0) array = new T[preAllocated];
}

template <typename T>
DynamicArray<T>::DynamicArray(T* array, uint32_t arraySize)
    : count(arraySize), size(arraySize), preAllocated(0), array(nullptr), TtoStringFunction(nullptr)
{
    if (arraySize == 0) return;
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

template <typename T>
DynamicArray<T>::DynamicArray(const DynamicArray<T>& other)
    : DynamicArray(0)
{
    operator=(other);
}

template <typename T>
bool DynamicArray<T>::resize(uint32_t newSize)
{
    T* newArray = nullptr;
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

    newArray = new T[newSize];
    if (newArray == nullptr) goto ErrorOccured;
    for (uint32_t i = 0; i < count; i++) newArray[i] = array[i];
    delete[] array;
    array = newArray;
    size = newSize;
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
    if (count == size) if (!resize(size + 1)) return false;
    array[count] = value;
    count++;
    return true;
}

template <typename T>
bool DynamicArray<T>::append(DynamicArray<T> other)
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
bool DynamicArray<T>::push(DynamicArray<T> other)
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
    if (count == 0) return append(value);
    if (count == size) if (!resize(size + 1)) return false;
    for (uint32_t i = count - 1; i == 0; i--)
    {
        array[i + 1] = array[i];
    }
    array[0] = value;
    count++;
    return true;
}

template <typename T>
bool DynamicArray<T>::prepend(DynamicArray<T> other)
{
    return prepend(other.array, other.count);
}

template <typename T>
bool DynamicArray<T>::prepend(T* array, uint32_t arraySize)
{
    if (count + arraySize > size) if (!resize(count + arraySize)) return false;
    for (uint32_t i = count - 1; i >= arraySize; i--) this->array[i + arraySize] = this->array[i];
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
bool DynamicArray<T>::swap(uint32_t indexA, uint32_t indexB)
{
    if (indexA >= count || indexB >= count)
#ifdef DynamicArray_EXCEPT
    throw DynamicArrayException();
#else
    return false;
#endif

    T temp = array[indexA];
    array[indexA] = array[indexB];
    array[indexB] = temp;
    return true;
}

template <typename T>
T DynamicArray<T>::replace(T value, uint32_t index)
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
bool DynamicArray<T>::set(T* array, uint32_t arraySize)
{
    erase();
    if (!resize(arraySize)) return false;
    for (uint32_t i = 0; i < arraySize; i++) this->array[i] = array[i];
    count = arraySize;
    return true;
}

template <typename T>
void DynamicArray<T>::setPointer(T* array, uint32_t allocation, uint32_t count)
{
    delete[] this->array;
    this->array = array;
    preAllocated = allocation;
    this->count = count;
}

template <typename T>
bool DynamicArray<T>::remove(uint32_t index)
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
T* const DynamicArray<T>::get(uint32_t index)
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
str_type DynamicArray<T>::toString(str_type start, str_type del, str_type end, str_type (*TtoString)(const T&)) const
{
    str_type out = start;

    for (uint32_t i = 0; i < count; i++)
    {
        str_type Tstring;
        if (TtoString)
        {
            Tstring += TtoString(array[i]);
        }
        else if (TtoStringFunction)
        {
            Tstring += TtoStringFunction(array[i]);
        }
        else
        {
#ifdef USE_SS
            std::stringstream stream = std::stringstream();
            stream << array[i];
            Tstring += stream.str();
#else
            Tstring += array[i];
#endif
        }

        if (i != count - 1) Tstring += del;
        out += Tstring;
    }
    out += end;
    return out;
}

template <typename T>
DynamicArray<T>& DynamicArray<T>::operator=(const DynamicArray<T>& other)
{
    delete[] array;
    set(other.array, other.count);
    return *this;
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
DynamicArray<T>& DynamicArray<T>::operator+=(T value)
{
    append(value);
    return *this;
}

template <typename T>
DynamicArray<T>& DynamicArray<T>::operator+=(DynamicArray<T> other)
{
    return operator+=(other);
}

template <typename T>
DynamicArray<T>& DynamicArray<T>::operator+=(DynamicArray<T>& other)
{
    join(other);
    return *this;
}

template <typename T>
DynamicArray<T>& DynamicArray<T>::operator+(const DynamicArray<T> other) const
{
    return operator+(other);
}

template <typename T>
DynamicArray<T>& DynamicArray<T>::operator+(const DynamicArray<T>& other) const
{
    DynamicArray<T> newArray = DynamicArray();
    newArray += *this;
    newArray += other;
    return newArray;
}

template <typename T>
bool DynamicArray<T>::operator==(DynamicArray<T>& other) const
{
    if (other.count != count) return false;
    for (uint32_t i = 0; i < count; i++) if (array[i] != other.array[i]) return false;
    return true;
}

template <typename T>
bool DynamicArray<T>::operator!=(DynamicArray<T>& other) const
{
    return !operator==(other);
}

template <typename T>
bool DynamicArray<T>::operator>(DynamicArray<T>& other) const
{
    return (count > other.count);
}

template <typename T>
bool DynamicArray<T>::operator<(DynamicArray<T>& other) const
{
    return (count < other.count);
}

template <typename T>
bool DynamicArray<T>::operator>=(DynamicArray<T>& other) const
{
    return (count >= other.count);
}

template <typename T>
bool DynamicArray<T>::operator<=(DynamicArray<T>& other) const
{
    return (count <= other.count);
}

template <typename T>
DynamicArray<T> operator+(const DynamicArray<T>& lhs, const DynamicArray<T>& rhs)
{
    DynamicArray<T> result = DynamicArray<T>(lhs.array, lhs.count);
    result += rhs;
    return result;
}

template <typename T> 
DynamicArray<T>::operator T*() const
{
    return array;
}

template <typename T>
DynamicArray<T>::operator str_type() const
{
    return toString();
}

template <typename T>
DynamicArrayRange<T> DynamicArray<T>::begin()
{
    return DynamicArrayRange<T>(0, this);
}

template <typename T>
DynamicArrayRange<T> DynamicArray<T>::end()
{
    return DynamicArrayRange<T>(count - 1, this);
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
#pragma endregion

#pragma region DynamicArrayRange
template <typename T>
DynamicArrayRange<T>::DynamicArrayRange(uint32_t index, DynamicArray<T>* array)
    : index(index), array(array)
{
}

template <typename T>
bool DynamicArrayRange<T>::operator!=(DynamicArrayRange<T> other)
{
    return (index != other.index);
}

template <typename T>
T& DynamicArrayRange<T>::operator*()
{
    return array->array[index];
}

template <typename T>
void DynamicArrayRange<T>::operator++()
{
    index++;
}

template <typename T>
uint32_t DynamicArrayRange<T>::getIndex()
{
    return index;
}

template <typename T>
DynamicArrayRange<T>::~DynamicArrayRange()
{
}
#pragma endregion

#undef str_type