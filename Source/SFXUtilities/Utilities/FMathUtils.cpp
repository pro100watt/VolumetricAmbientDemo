#include "FMathUtils.h"

#include "Math/Vector2D.h"

namespace Utils
{
	namespace FMathExt
	{
		bool IsInsideTriangle2D(const FVector2D& A, const FVector2D& B, const FVector2D& C, const FVector2D& Point)
		{
			FVector2D ALocal = A - C;
			FVector2D BLocal = B - C;
			FVector2D PointLocal = Point - C;

			return IsInsideTriangleLocal2D(ALocal, BLocal, PointLocal);
		}

		bool IsInsideTriangleLocal2D(const FVector2D& A, const FVector2D& B, const FVector2D& Point)
		{
			float LCross = A ^ Point;
			float RCross = Point ^ B;

			ensure(LCross >= 0.f && RCross >= 0.f);

			float InvHalfArea = 1.f / (A ^ B);

			return 1.f >= InvHalfArea * (LCross + RCross);
		}
	}
}