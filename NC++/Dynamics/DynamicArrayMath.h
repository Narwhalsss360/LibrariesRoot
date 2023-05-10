#pragma once

#include "DynamicArray.h"

namespace DynamicArrayMath
{
	template <typename T>
	struct EnumeratedItem
	{
		int64_t index;
		T& item;

		bool valid() { return (index != -1); }
	};

	template <typename T>
	EnumeratedItem<T> min(DynamicArray<T>& dynamicArray, bool (*comparator)(T&, T&) = nullptr)
	{
		if (dynamicArray.getCount() == 1) return { -1, T() };
		int64_t index;
		T& minItem;

		for (uint32_t i = 0; i < dynamicArray.getCount(); i++)
		{
			if (comparator)
			{
				if (comparator(dynamicArray[i], minItem))
				{
					minItem = dynamicArray[i];
					index = i;
				}
			}
			else
			{
				if (dynamicArray[i] < minItem)
				{
					minItem = dynamicArray[i];
					index = i;
				}
			}
		}

		return { index, dynamicArray[index] };
	}

	template <typename T>
	EnumeratedItem<T> max(DynamicArray<T>& dynamicArray, bool (*comparator)(T&, T&) = nullptr)
	{
		if (dynamicArray.getCount() == 1) return { -1, T() };
		int64_t index;
		T& maxItem;

		for (uint32_t i = 0; i < dynamicArray.getCount(); i++)
		{
			if (comparator)
			{
				if (comparator(dynamicArray[i], maxItem))
				{
					maxItem = dynamicArray[i];
					index = i;
				}
			}
			else
			{
				if (dynamicArray[i] > maxItem)
				{
					maxItem = dynamicArray[i];
					index = i;
				}
			}
		}

		return { index, dynamicArray[index] };
	}

	template <typename T>
	T product(DynamicArray<T>& numbers)
	{
		T p = 1;
		for (T& number : numbers) p *= number;
		return p;
	}

	template <typename T>
	T sum(DynamicArray<T> dynamicArray)
	{
		T sum;
		for (T num : dynamicArray) sum += num;
		return sum;
	}

	template <typename T>
	T average(DynamicArray<T>& dynamicArray)
	{
		int total = sum(dynamicArray);
		return sum(dynamicArray) / dynamicArray.getCount();
	}

	template <typename T>
	T median(DynamicArray<T>& numbers)
	{
		if (numbers.getCount() % 2 == 0)
			return numbers[numbers.getCount() / 2];
		else
			return (numbers[numbers.getCount() / 2] + numbers[(numbers.getCount() / 2) - 1]) / 2;
	}

	template <typename T>
	DynamicArray<T> unique(DynamicArray<T>& items)
	{
		DynamicArray<T> uniqueItems;
		for (T& item : items) if (!uniqueItems.has(item)) uniqueItems.append(item);
		return uniqueItems;
	}

	template <typename T>
	void shuffle(DynamicArray<T>& numbers)
	{
		for (size_t i = 0; i < numbers.getCount(); i++)
		{
			if (rand() % 100 > 49)
			{
				numbers.swap(i, rand() % i - 0);
			}
		}
	}

	template <typename T>
	DynamicArray<T> primeFactors(T number)
	{
		DynamicArray<T> factors;
		T divisor = 2;

		while (number > 1)
		{
			if (number % divisor == 0)
			{
				factors += divisor;
				number /= divisor;
			}
			else
			{
				divisor++;
			}
		}

		return factors;
	}

	template <typename T>
	DynamicArray<DynamicArray<T>> factors(T number)
	{
		DynamicArray<DynamicArray<T>> factors;
		uint32_t currentIdx = 0;
		for (T i = 1; i <= number; i++)
		{
			if (number % i == 0)
			{
				factors.append(DynamicArray<T>());
				factors[currentIdx].append(i);
				factors[currentIdx].append(number / i);
				currentIdx++;
			}
		}

		return factors;
	}

	template <typename T>
	T LCM(DynamicArray<T>& numbers)
	{
		DynamicArray<DynamicArray<T>> number_primeFactors;
		DynamicArray<T> lcm_factors;
		DynamicArray<T> lcm_factors_exp;

		for (T& number : numbers)
		{
			DynamicArray<T> primeFactorsOfCurrent = primeFactors(number);
			number_primeFactors.append(primeFactorsOfCurrent);
			lcm_factors.append(primeFactorsOfCurrent);
		}

		lcm_factors = unique(lcm_factors);
		lcm_factors_exp.resize(lcm_factors.getCount());

		for (uint32_t iFactor = 0; iFactor < lcm_factors.getCount(); iFactor++)
		{
			for (uint32_t iList = 0; iList < number_primeFactors.getCount(); iList++)
			{
				T countOfCurrentFactor = number_primeFactors[iList].countOf(lcm_factors[iFactor]);
				if (countOfCurrentFactor > lcm_factors_exp[iFactor])
				{
					lcm_factors_exp[iFactor] = countOfCurrentFactor;
				}
			}
		}

		T lcm = 1;

		for (uint32_t iFactor = 0; iFactor < lcm_factors.getCount(); iFactor++)
		{
			lcm *= pow(lcm_factors[iFactor], lcm_factors_exp[iFactor]);
		}

		return lcm;
	}

	template <typename T>
	T GCD(DynamicArray<T>& numbers)
	{
		return product(numbers) / LCM(numbers);
	}

	template <typename T>
	T standardDeviation(DynamicArray<T>& dynamicArray)
	{
		T summation = 0;
		T avg = average(dynamicArray);
		for (auto& num : dynamicArray) summation += pow(num - avg, 2);
		return sqrt(summation / dynamicArray.getCount());
	}

	template <typename T>
	void reverse(DynamicArray<T>& dynamicArray)
	{
		if (dynamicArray.getCount() <= 1 ) return;

		T* pointer = dynamicArray.get(0);
		for (uint32_t i = 0; i < int(dynamicArray.getCount() / 2); i++)
		{
			T temp = pointer[i];
			pointer[i] = pointer[dynamicArray.getCount() - 1 - i];
			pointer[dynamicArray.getCount() - 1 - i] = temp;
		}
	}

	template <typename T>
	void bubbleSort(DynamicArray<T>& dynamicArray, int (*comparator)(T&, T&) = nullptr)
	{
		T* pointer = dynamicArray.get(0);
		for (size_t iOuter = 0; iOuter < dynamicArray.getCount(); iOuter++)
		{
			for (uint32_t iInner = 0; iInner < dynamicArray.getCount() - 1 - iOuter; iInner++)
			{
				if (comparator)
				{
					if (comparator(pointer[iInner], pointer[iInner + 1]) > 0) dynamicArray.swap(iInner, iInner + 1);
				}
				else
				{
					if (pointer[iInner] > pointer[iInner + 1]) dynamicArray.swap(iInner, iInner + 1);
				}
			}
		}
	}
}