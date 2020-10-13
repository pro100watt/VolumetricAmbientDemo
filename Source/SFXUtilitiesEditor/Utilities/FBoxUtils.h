#pragma once

struct FVector2D;
struct FVector;
struct FBox2D;

namespace Utils
{
	FBox2D& operator+=(FBox2D& This, const FVector2D& Other);
	void Inscribe(FBox2D& Box, const FVector2D* Polygon, const int32 Num);
}