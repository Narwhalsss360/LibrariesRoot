#pragma once

#define static_for(type, collection) for (size_t index_##collection = 0; index_##collection < sizeof(collection) / sizeof(type); index_##collection++)

template <typename _Type>
class RangeIterator
{
public:
	RangeIterator(_Type* const array, size_t currentIndex = 0)
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
class Iterable
{
public:
	Iterable(_Type* array)
		: m_Array(array)
	{
	}

	inline RangeIterator<_Type> begin() const
	{
		return RangeIterator<_Type>(m_Array);
	}

	inline const RangeIterator<_Type> end() const
	{
		return RangeIterator<_Type>(m_Array, _Size);
	}
private:
	_Type* const m_Array;
};

template <typename _Type>
struct EnumeratedItem
{
	_Type& item;
	size_t index;

	_Type& operator*()
	{
		return item;
	}
};

template <typename _Type>
class EnumeratedRangeIterator
{
public:
	EnumeratedRangeIterator(_Type* const array, size_t currentIndex = 0)
		: m_Array(array), m_CurrentIndex(currentIndex)
	{
	}

	inline bool operator!=(const EnumeratedRangeIterator<_Type>& other) const
	{
		return m_CurrentIndex != other.m_CurrentIndex;
	}

	inline EnumeratedItem<_Type> operator*() const
	{
		return { m_Array[m_CurrentIndex], m_CurrentIndex };
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
class EnumeratedIterable
{
public:
	EnumeratedIterable(_Type* array)
		: m_Array(array)
	{
	}

	inline EnumeratedRangeIterator<_Type> begin() const
	{
		return EnumeratedRangeIterator<_Type>(m_Array);
	}

	inline const EnumeratedRangeIterator<_Type> end() const
	{
		return EnumeratedRangeIterator<_Type>(m_Array, _Size);
	}
private:
	_Type* const m_Array;
};

template <typename _Type, size_t _Size>
inline Iterable<_Type, _Size> Iterate(_Type* array)
{
	return Iterable<_Type, _Size>(array);
}

template <typename _Type, size_t _Size>
inline EnumeratedIterable<_Type, _Size> Enumerate(_Type* array)
{
	return EnumeratedIterable<_Type, _Size>(array);
}