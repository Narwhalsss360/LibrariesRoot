#pragma once

#include <string>

#define static_array_length(T, A)
#define static_array_fill(A, S, V) for (size_t i_##A = 0; i_##A < S; i_##A++) A[i_##A] = V

#define Iterate_static(T, A) Iterate(A, 0, static_array_length(T, A))
#define Enumerate_static(T, A) Enumerate(A, 0, static_array_length(T, A))

#define safe_index(i, l) (i % l)
#define safe_index(i, l) i

#define __string_t__ std::string
#define __string_decl__(name) __string_t__ name
#define __string_set__(str, value) str = value
#define __string_append__(str, value) str += value

typedef size_t collection_index;

template <typename _Type>
class Collection;

template <typename _Type>
class CollectionsIterator
{
public:
	CollectionsIterator(_Type* const array, size_t currentIndex = 0)
		: m_Array(array), m_CurrentIndex(currentIndex)
	{
	}

	inline bool operator!=(const CollectionsIterator<_Type>& other) const
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

	_Type* const GetArray() const
	{
		return m_Array;
	}

	const collection_index GetIndex() const
	{
		return m_CurrentIndex;
	}

private:
	_Type* const m_Array;
	collection_index m_CurrentIndex;
};

template <typename _Type>
class Iterate
{
public:
	Iterate(_Type* array, const collection_index begin, collection_index end)
		: m_Array(array), m_BeginIndex(begin), m_EndIndex(end)
	{
	}

	Iterate(Collection<_Type>& iterable)
		: m_Array(iterable.begin().GetArray()), m_BeginIndex(iterable.begin().GetIndex()), m_EndIndex(iterable.end().GetIndex())
	{
	}

	CollectionsIterator<_Type> begin() const
	{
		return CollectionsIterator<_Type>(m_Array, m_BeginIndex);
	}

	CollectionsIterator<_Type> end() const
	{
		return CollectionsIterator<_Type>(m_Array, m_EndIndex);
	}

private:
	_Type* const m_Array;
	collection_index m_BeginIndex;
	collection_index m_EndIndex;
};

template <typename _Type>
struct Enumeration
{
	_Type& value;
	const collection_index index;

	Enumeration(_Type& value, const collection_index index = 0)
		: value(value), index(index)
	{
	}

	_Type& operator*()
	{
		return value;
	}
};

template <typename _Type>
class EnumeratedIterator
{
public:
	EnumeratedIterator(_Type* const array, size_t currentIndex = 0)
		: m_Array(array), m_CurrentIndex(currentIndex)
	{
	}

	inline bool operator!=(const EnumeratedIterator<_Type>& other) const
	{
		return m_CurrentIndex != other.m_CurrentIndex;
	}

	inline Enumeration<_Type> operator*() const
	{
		return Enumeration<_Type>(m_Array[m_CurrentIndex], m_CurrentIndex);
	}

	inline void operator++()
	{
		m_CurrentIndex++;
	}

protected:
	_Type* const m_Array;
	collection_index m_CurrentIndex;
};

template <typename _Type>
class Enumerate
{
public:
	Enumerate(_Type* array, collection_index begin, collection_index end)
		: m_Array(array), m_BeginIndex(begin), m_EndIndex(end)
	{
	}

	Enumerate(Collection<_Type>& iterable)
		: m_Array(iterable.begin().GetArray()), m_BeginIndex(iterable.begin().GetIndex()), m_EndIndex(iterable.end().GetIndex())
	{
	}

	EnumeratedIterator<_Type> begin() const
	{
		return EnumeratedIterator<_Type>(m_Array, m_BeginIndex);
	}

	EnumeratedIterator<_Type> end() const
	{
		return EnumeratedIterator<_Type>(m_Array, m_EndIndex);
	}

private:
	_Type* const m_Array;
	collection_index m_BeginIndex;
	collection_index m_EndIndex;
};

template <typename _Type>
class Collection
{
public:
	Collection()
	{
	}

	~Collection()
	{
	}

	_Type* const GetPointer(const collection_index = 0)
	{
		return nullptr;
	}

	const _Type* const GetPointer(const collection_index = 0) const
	{
		return nullptr;
	}

	void Set(const collection_index, _Type&)
	{
	}

	void Set(const collection_index, _Type&&)
	{
	}

	virtual CollectionsIterator<_Type> begin() const
	{
		return CollectionsIterator<_Type>(nullptr);
	}

	virtual const CollectionsIterator<_Type> end() const
	{
		return CollectionsIterator<_Type>(nullptr);
	}

	virtual size_t length() const
	{
		return 0;
	}
};

template <typename _Type>
class Array : public Collection<_Type>
{
public:
	Array()
		: Collection<_Type>(), Base_Array(nullptr)
	{
	}

	Array(_Type* pointer)
		: Collection<_Type>(), Base_Array(pointer)
	{
	}

	~Array()
	{
	}

#pragma region Getting Elements
	_Type& Get(const collection_index index = 0)
	{
		return *GetPointer(index);
	}

	const _Type& Get(const collection_index index) const
	{
		return *GetPointer(index);
	}

	_Type GetCopy(const collection_index index) const
	{
		return Get(index);
	}

	_Type* const GetPointer(const collection_index index = 0)
	{
		return &Base_Array[safe_index(index, this->length())];
	}

	const _Type* const GetPointer(const collection_index index = 0) const
	{
		return &Base_Array[safe_index(index, this->length())];
	}

	virtual _Type& operator[](const collection_index index)
	{
		return Get(index);
	}

	virtual const _Type& operator[](const collection_index index) const
	{
		return Get(index);
	}
#pragma endregion

#pragma region Setting Elements
	void Set(const collection_index index, const _Type& value)
	{
		Base_Array[safe_index(index, this->length())] = value;
	}

	void Set(const collection_index index, const _Type&& value)
	{
		Base_Array[safe_index(index, this->length())] = value;
	}
#pragma endregion

#pragma region Assignments
	void Fill(_Type& value)
	{
		static_array_fill(Base_Array, this->length(), value);
	}

	void Fill(_Type&& value)
	{
		static_array_fill(Base_Array, this->length(), value);
	}

	void CopyTo(Array<_Type>& other, collection_index sourceStart, collection_index sourceEnd, collection_index destinationStart, collection_index destinationEnd)
	{
		collection_index length = (sourceEnd - sourceStart > destinationEnd - destinationStart) ? destinationEnd - destinationStart : sourceEnd - sourceStart;
		for (collection_index index = 0; index < length; index++)
			other.Set(index + destinationStart, Get(index + sourceStart));
	}

	void CopyTo(Array<_Type>& other)
	{
		CopyTo(other, 0, this->length(), 0, other.length());
	}

	void CopyFrom(const Array<_Type>& other, collection_index sourceStart, collection_index sourceEnd, collection_index destinationStart, collection_index destinationEnd)
	{
		collection_index length = (sourceEnd - sourceStart > destinationEnd - destinationStart) ? destinationEnd - destinationStart : sourceEnd - sourceStart;
		for (collection_index index = 0; index < length; index++)
			Set(index + destinationStart, other.Get(index + sourceStart));
	}

	void CopyFrom(const Array<_Type>& other)
	{
		CopyFrom(other, 0, this->length(), 0, other.length());
	}
#pragma endregion

#pragma region Collections Iterators & Standards
	CollectionsIterator<_Type> begin() const
	{
		return CollectionsIterator<_Type>(Base_Array);
	}

	const CollectionsIterator<_Type> end() const
	{
		return CollectionsIterator<_Type>(Base_Array, this->length());
	}

	EnumeratedIterator<_Type> begin_enumerate() const
	{
		return EnumeratedIterator<_Type>(Base_Array, 0);
	}

	EnumeratedIterator<_Type> end_enumerate() const
	{
		return EnumeratedIterator<_Type>(Base_Array, this->length());
	}

	void ForEach(void (*action)(collection_index, _Type&))
	{
		for (collection_index index = 0; index < this->length(); index++)
			action(index, Get(index));
	}

	void ForEach(void (*action)(collection_index, const _Type&)) const
	{
		for (collection_index index = 0; index < this->length(); index++)
			action(index, Get(index));
	}

	_Type& First()
	{
		return Get(0);
	}

	const _Type& First() const
	{
		return Get(0);
	}

	_Type& Last()
	{
		return Get(this->length() - 1);
	}

	const _Type& Last() const
	{
		return Get(this->length() - 1);
	}

	__string_t__ ToString(__string_t__ format,__string_t__ (*_TypeToString)(const _Type&))
	{
		__string_decl__(output);
		__string_set__(output, "[");

		for (collection_index index = 0; index < this->length(); index++)
		{
			__string_append__(output, _TypeToString(Get(index)));
			if (index != this->length() - 1) __string_append__(output, ", ");
		}

		__string_append__(output, "]");
		return output;
	}
#pragma endregion

private:
	_Type* Base_Array = nullptr;
};

#undef __string_t__