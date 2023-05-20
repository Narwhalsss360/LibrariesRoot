#pragma once

template <typename _Type, size_t _Size>
class StaticArrayIterator
{
public:
	StaticArrayIterator(_Type* array, size_t currentIndex = 0)
		: m_Array(array), m_CurrentIndex(currentIndex)
	{
	}

	inline bool operator!=(StaticArrayIterator& other)
	{
		return m_CurrentIndex != other.m_CurrentIndex;
	}

	inline _Type& operator*()
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
class ArrayIterators
{
public:
	ArrayIterators(_Type* array)
		: m_Array(array)
	{
	}

	inline StaticArrayIterator<_Type, _Size> begin()
	{
		return StaticArrayIterator<_Type, _Size>(m_Array);
	}

	inline StaticArrayIterator<_Type, _Size> end()
	{
		return StaticArrayIterator<_Type, _Size>(m_Array, _Size);
	}
private:
	_Type* m_Array;
};

template <typename _Type, size_t _Size>
ArrayIterators<_Type, _Size> IterateArray(_Type* const array)
{
	return ArrayIterators<_Type, _Size>(array);
}