#pragma once

struct FVector2D;

namespace Utils
{
	namespace FMathExt
	{
		bool IsInsideTriangle2D(const FVector2D& A, const FVector2D& B, const FVector2D& C, const FVector2D& Point);
		bool IsInsideTriangleLocal2D(const FVector2D& A, const FVector2D& B, const FVector2D& Point);
	}
}