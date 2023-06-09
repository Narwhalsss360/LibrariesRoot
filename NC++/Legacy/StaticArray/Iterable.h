template<typename Col, typename Deref>
class _Iterator
{
public:
	_Iterator(Col& col, size_t idx = 0)
		: m_Col(col), idx(idx)
	{
	}

	inline bool operator!=(_Iterator<Col, Deref>& other)
	{
		return idx != other.idx;
	}

	inline void operator++()
	{
		idx++;
	}

	virtual Deref operator*()
	{
		return m_Col[idx];
	}

protected:
	mutable Col& m_Col;
	size_t idx;
};

template <typename T>
struct _Enumeration
{
	T& val;
	size_t idx;

	_Enumeration(T& val, size_t idx)
		: val(val), idx(idx)
	{
	}
};

template <typename T>
class _EnumerationIterator : public _Iterator<T, _Enumeration<T>>
{
	_Enumeration<T> operator*() override
	{
		T& val = m_Col[0];
		return _Enumeration<T>(this->m_Col[this->idx], this->idx);
	}
};