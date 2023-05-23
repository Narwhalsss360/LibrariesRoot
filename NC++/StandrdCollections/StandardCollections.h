#pragma once

template <typename Collectable>
class Collection
{
public:
	virtual inline size_t length() = 0;
	virtual Collectable& operator[](size_t) = 0;
	virtual const Collectable& operator[](size_t) const = 0;
};