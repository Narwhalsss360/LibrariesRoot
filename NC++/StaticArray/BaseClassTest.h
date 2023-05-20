#pragma once

typedef size_t array_index;

template <typename _Type>
class Array
{
public:
	Array();
	~Array();

	virtual _Type& operator[](array_index index)
	{
		return m_Array[index];
	}

	virtual const _Type& operator[](array_index index) const
	{
		return m_Array[index];
	}

private:
	_Type* m_Array;
};