#include "FBoxUtils.h"

#include "Math/UnrealMathUtility.h"
#include "Math/Box.h"

namespace Utils
{
	enum class ESide : unsigned int
	{
		PosX = 0b00u,
		PosY = 0b01u,
		NegX = 0b10u,
		NegY = 0b11u,
	};

	struct FCornerLine
	{
		int32 BeginIndex;
		int32 EndIndex;
		ESide BeginSide;
		ESide EndSide;
	};

	ESide GetSide(const FVector2D& Point);
	ESide NextCWSide(ESide Side);
	void ConstrainCorner(FBox2D& Box, const FCornerLine& Line, const FVector2D* Polygon);
	void ConstrainCorner(FVector2D& XPlane, FVector2D& YPlane, const FVector2D& LineBegin, const FVector2D& LineEnd);
	bool RadiusIntersection(const FVector2D& RadiusVector, const FVector2D& LineBegin, const FVector2D& LineEnd, FVector2D& OutIntersectionPoint);


	FORCEINLINE FBox2D& operator+=(FBox2D& This, const FVector2D& Other)
	{
		if (This.bIsValid)
		{
			This.Min.X = FMath::Min(This.Min.X, Other.X);
			This.Min.Y = FMath::Min(This.Min.Y, Other.Y);

			This.Max.X = FMath::Max(This.Max.X, Other.X);
			This.Max.Y = FMath::Max(This.Max.Y, Other.Y);
		}
		else
		{
			This.Min = This.Max = Other;
			This.bIsValid = 1;
		}

		return This;
	}

	void Inscribe(FBox2D& Box, const FVector2D* Polygon, const int32 Num)
	{
		TArray<FCornerLine> CornerLines;
		CornerLines.Reserve(4);

		{
			int32 LastIndex = Num - 1;
			ESide LastSide = GetSide(Polygon[LastIndex]);
			for (int32 Index = 0; Index < Num; Index++)
			{
				const FVector2D& Point = Polygon[Index];
				const ESide Side = GetSide(Point);
				switch (Side)
				{
				case ESide::PosX:
					Box.Max.X = FMath::Min(Box.Max.X, Point.X);
					break;
				case ESide::NegX:
					Box.Min.X = FMath::Max(Box.Min.X, Point.X);
					break;
				case ESide::PosY:
					Box.Max.Y = FMath::Min(Box.Max.Y, Point.Y);
					break;
				case ESide::NegY:
					Box.Min.Y = FMath::Max(Box.Min.Y, Point.Y);
					break;
				default:
					break;
				}

				if (Side != LastSide)
				{
					CornerLines.Add({ LastIndex, Index, LastSide, Side });
				}

				LastIndex = Index;
				LastSide = Side;
			}

			ensure(CornerLines.Num() <= 4);
		}

		for (FCornerLine& Line : CornerLines)
		{
			ConstrainCorner(Box, Line, Polygon);

			ESide NextSide = NextCWSide(Line.BeginSide);
			if (NextSide != Line.EndSide)
			{
				ensure(NextCWSide(NextSide) == Line.EndSide);

				Line.BeginSide = NextSide;
				ConstrainCorner(Box, Line, Polygon);
			}
		}
	}

	ESide GetSide(const FVector2D& Point)
	{
		const bool bNegXPosY = Point.Y > Point.X;
		const bool bNegXNegY = Point.Y < -Point.X;
		return ESide((bNegXPosY ? 0b01 : 0b00) ^ (bNegXNegY ? 0b11 : 0b00));
	}

	ESide NextCWSide(ESide Side)
	{
		return static_cast<ESide>((static_cast<unsigned int>(Side) + 1u) & 0b11u);
	}

	void ConstrainCorner(FBox2D& Box, const FCornerLine& Line, const FVector2D* Polygon)
	{
		const FVector2D& LineBegin = Polygon[Line.BeginIndex];
		const FVector2D& LineEnd = Polygon[Line.EndIndex];

		switch (Line.BeginSide)
		{
		case ESide::PosX:
			ConstrainCorner(Box.Max, Box.Max, LineBegin, LineEnd);
			break;
		case ESide::PosY:
			ConstrainCorner(Box.Min, Box.Max, LineBegin, LineEnd);
			break;
		case ESide::NegX:
			ConstrainCorner(Box.Min, Box.Min, LineBegin, LineEnd);
			break;
		case ESide::NegY:
			ConstrainCorner(Box.Max, Box.Min, LineBegin, LineEnd);
			break;
		default:
			break;
		}
	}

	void ConstrainCorner(FVector2D& XPlane, FVector2D& YPlane, const FVector2D& LineBegin, const FVector2D& LineEnd)
	{
		FVector2D Corner(XPlane.X, YPlane.Y);

		FVector2D NewCorner;
		if (RadiusIntersection(Corner, LineBegin, LineEnd, OUT NewCorner))
		{
			XPlane.X = NewCorner.X;
			YPlane.Y = NewCorner.Y;
		}
	}

	bool RadiusIntersection(const FVector2D& RadiusVector, const FVector2D& LineBegin, const FVector2D& LineEnd, FVector2D& OutIntersectionPoint)
	{
		const FVector2D& R = RadiusVector;
		const FVector2D V = LineEnd - LineBegin;

		const float Cross = R ^ V;
		const float S = (LineBegin ^ R) / Cross;
		const float T = (LineBegin ^ V) / Cross;

		const bool bIntersects = (S >= 0 && S <= 1 && T >= 0 && T <= 1);

		if (bIntersects)
		{
			OutIntersectionPoint = T * R;
		}

		return bIntersects;
	}
}