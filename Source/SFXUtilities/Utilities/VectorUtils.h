#pragma once

#include "Math/Vector.h"
#include "Math/Vector2D.h"

namespace Utils
{
	const FVector2D& As2D(const FVector& V)
	{
		return *reinterpret_cast<const FVector2D*>(&V.X);
	}

	FVector2D& As2D(FVector& V)
	{
		return *reinterpret_cast<FVector2D*>(&V.X);
	}

	FVector To3D(const FVector2D& V)
	{
		return FVector(V, 0.f);
	}
}