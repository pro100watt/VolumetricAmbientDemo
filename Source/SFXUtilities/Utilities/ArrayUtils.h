#pragma once

#include "Containers/Array.h"

namespace Utils
{
	template<class T>
	struct TCyclicArray
	{
		int32 Next(int32 Index)
		{
			check(Index >= 0 && Index < Array.Num());
			return (++Index == Array.Num()) ? 0 : Index;
		}

		int32 Prev(int32 Index)
		{
			check(Index >= 0 && Index < Array.Num());
			return (Index == 0) ? Array.Num() - 1 : Index - 1;
		}

		const TArray<T>& Array;
	};

	template<class T>
	TCyclicArray<T> GetCyclic(const TArray<T>& Array) { return TCyclicArray<T>{ Array }; }
}