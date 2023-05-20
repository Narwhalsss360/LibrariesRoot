#pragma once

#include "StaticArrayIterators.h"

template <typename _Type, size_t _Size>
class StaticArray
{
public:
	StaticArray()
	{
	}

	StaticArray(_Type& fill)
		: m_Array{fill}
	{
	}

	StaticArray(_Type&& fill)
		: m_Array{fill}
	{
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

	_Type* begin()
	{
		return &First();
	}

	_Type* end()
	{
		return &Last();
	}

	const _Type* begin() const
	{
		return &First();
	}

	const _Type* end() const
	{
		return &Last();
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
private:
	_Type m_Array[_Size];
};

template <typename _Type, size_t _Size>
ArrayIterators<_Type, _Size> IterateArray(StaticArray<_Type, _Size>& staticArray)
{
	return ArrayIterators<_Type, _Size>(staticArray.GetPointer());
}