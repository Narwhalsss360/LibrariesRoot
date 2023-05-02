#pragma once

#include <DynamicArray.h>

namespace DynamicArrayUtility
{
	template <typename T>
	void reverse(DynamicArray<T>& dArray)
	{
		if (dArray.getCount() == 0) return;

		T* pointer = dArray.get(0);
		for (uint32_t i = 0; i < int(dArray.getCount() / 2); i++)
		{
			T temp = pointer[i];
			pointer[i] = pointer[dArray.getCount() - 1 - i];
			pointer[dArray.getCount() - 1 - i] = temp;
		}
	}

	template <typename T>
	void bubbleSort(DynamicArray<T>& dArray, int (*comparator)(T&, T&) = nullptr)
	{
		T* pointer = dArray.get(0);
		for (size_t iOuter = 0; iOuter < dArray.getCount(); iOuter++)
		{
			for (uint32_t iInner = 0; iInner < dArray.getCount() - 1 - iOuter; iInner++)
			{
				if (comparator)
				{
					if (comparator(pointer[iInner], pointer[iInner + 1]) > 0) dArray.swap(iInner, iInner + 1);
				}
				else
				{
					if (pointer[iInner] > pointer[iInner + 1]) dArray.swap(iInner, iInner + 1);
				}
			}
		}
	}
}