#pragma once

#include "Iterable.h"
#include <initializer_list>
#include <stdarg.h>

#define StaticArrayInit(t, n, ...) \
	constexpr t init_array_##n[] = __VA_ARGS__; \
	StaticArray<t, sizeof(init_array_##n) / sizeof(t)> n(init_array_##n, sizeof(init_array_##n) / sizeof(t));

template <typename _Type>
class StaticArrayIterator
{
public:
	StaticArrayIterator(_Type* const array, size_t currentIndex = 0)
		: m_Array(array), m_CurrentIndex(currentIndex)
	{
	}

	inline bool operator!=(const RangeIterator<_Type>& other) const
	{
		return m_CurrentIndex != other.m_CurrentIndex;
	}

	inline _Type& operator*() const
	{
		return m_Array[m_CurrentIndex];
	}

	inline void operator++()
	{
		m_CurrentIndex++;
	}

private:
	_Type* const m_Array;
	size_t m_CurrentIndex;
};

template <typename _Type, size_t _Size>
class StaticArray
{
public:
	StaticArray()
	{
	}

	StaticArray(_Type& fill)
		: m_Array{ fill }
	{
	}

	StaticArray(_Type&& fill)
		: m_Array{ fill }
	{
	}

	StaticArray(_Type* const array, const size_t arraySize)
		: m_Array{ _Type() }
	{
		for (size_t index = 0; index < (_Size > arraySize ? arraySize : _Size); index++)
			m_Array[index] = array[index];
	}

	StaticArray(const _Type* const array, const size_t arraySize)
		: m_Array{ _Type() }
	{
		for (size_t index = 0; index < (_Size > arraySize ? arraySize : _Size); index++)
			m_Array[index] = array[index];
	}

	StaticArray(const std::initializer_list<_Type> iList)
		: m_Array{ _Type() }
	{
		operator=(iList);
	}

	StaticArray(...)
		: m_Array{ _Type() }
	{
		va_list arguments;
		va_start(arguments, _Size);
		if (!va_arg(arguments, bool)) return;
		for (size_t index = 0; index < _Size; index++)
			m_Array[index] = va_arg(arguments, _Type);
	}

	~StaticArray()
	{
	}

	_Type& Get(size_t index)
	{
		return m_Array[index % _Size];
	}

	_Type GetCopy(size_t index)
	{
		return m_Array[index % _Size];
	}

	_Type* GetPointer(size_t index = 0)
	{
		return &m_Array[index % _Size];
	}

	const _Type& Get(size_t index) const
	{
		return m_Array[index % _Size];
	}

	const _Type GetCopy(size_t index) const
	{
		return m_Array[index % _Size];
	}

	const _Type* GetPointer(size_t index = 0) const
	{
		return &m_Array[index % _Size];
	}

	void Set(size_t index, _Type& value)
	{
		m_Array[index % _Size] = value;
	}

	void Set(size_t index, _Type&& value)
	{
		m_Array[index % _Size] = value;
	}

	_Type& First()
	{
		return m_Array[0];
	}

	const _Type& First() const
	{
		return m_Array[0];
	}

	_Type& Last()
	{
		return m_Array[_Size - 1];
	}

	const _Type& Last() const
	{
		return m_Array[_Size - 1];
	}

	constexpr inline size_t Size() const
	{
		return _Size;
	}

	_Type& operator[](size_t index)
	{
		return Get(index);
	}

	const _Type& operator[](size_t index) const
	{
		return Get(index);
	}

	RangeIterator<_Type> begin()
	{
		return RangeIterator<_Type>(m_Array);
	}

	const RangeIterator<_Type> end()
	{
		return RangeIterator<_Type>(m_Array, _Size);
	}

	StaticArrayIterator<_Type> begin() const
	{
		return StaticArrayIterator<_Type>(m_Array);
	}

	const StaticArrayIterator<_Type> end() const
	{
		return StaticArrayIterator<_Type>(m_Array, _Size);
	}

	void ForEach(void (*f)(size_t, _Type&))
	{
		for (size_t index = 0; index < _Size - 1; index++)
			f(index, m_Array[index]);
	}

	void ForEach(void (*f)(size_t, const _Type&)) const
	{
		for (size_t index = 0; index < _Size - 1; index++)
			f(index, m_Array[index]);
	}

	operator _Type* ()
	{
		return m_Array;
	}

	StaticArray& operator=(const std::initializer_list<_Type> iList)
	{
		for (size_t index = 0; index < (_Size > iList.size() ? iList.size() : _Size); index++)
			m_Array[index] = iList.begin()[index];
		return *this;
	}
private:
	_Type m_Array[_Size];
};