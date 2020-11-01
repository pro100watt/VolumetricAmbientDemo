// Fill out your copyright notice in the Description page of Project Settings.

#include "PolygonArea2DComponent.h"

#include "SFXUtilities/Utilities/ArrayUtils.h"
#include "SFXUtilities/Utilities/FMathUtils.h"
#include "SFXUtilities/Utilities/VectorUtils.h"

#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif

namespace
{
	/** Returns index of any element X such that Pred(X) == true (or INDEX_NONE if not found) */
	template<class T, class Pred>
	int32 FindAnyPoint(const TArray<T>& Points, Pred IsOK)
	{
		int32 N = Points.Num();

		int32 Step = 1;
		while (Step <= N) Step *= 2;

		for (; Step > 1; Step /= 2)
		{
			for (int32 i = Step / 2 - 1; i < N; i += Step)
			{
				if (IsOK(Points[i])) return i;
			}
		}

		return INDEX_NONE;
	}

	/**
	 * Returns true, if closest point on the next polygon line can potentially be closer than the current one
	 * If returns true, OutBestPossibleClosestDistSqr contains best possible squared distance to the next line
	 */
	bool IsNeedToCheckNextLine(const FVector2D& Location, const FVector2D& NextLineBegin, float ClosestPointDistSqr, float& OutBestPossibleClosestDistSqr)
	{
		check(!NextLineBegin.IsNearlyZero());

		const float Dot1 = Location | NextLineBegin;
		if (Dot1 <= 0.f)
		{
			// Distance to the origin is less than distance to PointVector, definitely no need to check further
			return false;
		}

		const float Dot2 = NextLineBegin | NextLineBegin;
		if (Dot1 < Dot2)
		{
			// Location to PointVector projection length is less than PointVector length
			// Distance to any next closest point can't be less than current ClosestPointDistSqr
			return false;
		}

		// Calculate the best closest distance, which may be produced by next point
		OutBestPossibleClosestDistSqr = (NextLineBegin * (Dot1 / Dot2) - Location).SizeSquared();

		// If the best closest distance is not better than current closest distance
		// we don't need to check further
		return ClosestPointDistSqr > OutBestPossibleClosestDistSqr;
	}
}

// Sets default values for this component's properties
UPolygonArea2DComponent::UPolygonArea2DComponent()
	: MinBox(FVector2D(-150.f), FVector2D(150.f))
	, MaxBox(FVector2D(-300.f), FVector2D(300.f))
#if WITH_EDITOR
	, EditorSelectedColor(FLinearColor::Red)
	, EditorUnselectedColor(FLinearColor::Green)
	, EditorBoxColor(FLinearColor::Yellow)
	, MinRadius(50.f)
	, bDrawArea(true)
	, bDrawBoxes(true)
	, bDrawTestedSegments(true)
#endif
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

#if WITH_EDITOR
	Points.Add(FVector2D(300.f, 0.f));
	Points.Add(FVector2D(0.f, 300.f));
	Points.Add(FVector2D(-300.f, 0.f));
	Points.Add(FVector2D(0.f, -300.f));
#endif
}


// Called when the game starts
void UPolygonArea2DComponent::BeginPlay()
{
	Super::BeginPlay();

	ensure(Points.Num() > 3);
}


// Called every frame
void UPolygonArea2DComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if WITH_EDITOR
	{
		using namespace Utils;

		FVector ActorLocation = GetOwner()->GetActorLocation();

		if (bDrawBoxes)
		{
			DrawDebugBox(GetWorld(),
				ActorLocation + To3D(MaxBox.GetCenter()),
				FVector(MaxBox.GetExtent(), 200.f), FColor::Red,
				false, -1., (uint8)1u, 5.f);

			DrawDebugBox(GetWorld(),
				ActorLocation + To3D(MinBox.GetCenter()),
				FVector(MinBox.GetExtent(), 200.f), FColor::Green,
				false, -1., (uint8)1u, 5.f);
		}

		if (bDrawArea)
		{
			FVector2D LastPoint = Points.Last();
			for (const auto& Point : Points)
			{
				DrawDebugLine(GetWorld(),
					ActorLocation + To3D(LastPoint),
					ActorLocation + To3D(Point),
					FColor::Yellow, false, -1., (uint8)1u, 5.f);

				LastPoint = Point;
			}
		}
	}
#endif
}

bool UPolygonArea2DComponent::IsWithinRadius(const FVector& Location, float Radius)
{
	using namespace Utils;
	return FVector2D::DistSquared(MaxBox.GetClosestPointTo(As2D(Location)), As2D(Location)) <= Radius * Radius;
}

FVector UPolygonArea2DComponent::FindClosestPoint(const FVector& Location)
{
	using namespace Utils;

	const FVector2D &Loc2D = As2D(Location);

	if (MinBox.IsInside(Loc2D))
	{
		// Location is inside the MinBox inscribed in polygon
		return Location;
	}

	auto PointsC = GetCyclic(Points);

	// Find line point indices of polygon sector containing the Loc2D
	int32 LeftIdx = FindContainingSector(Loc2D);
	int32 RightIdx = PointsC.Next(LeftIdx);

	const FVector2D &LeftPoint = Points[LeftIdx];
	const FVector2D &RightPoint = Points[RightIdx];

#if WITH_EDITOR
	DrawDebugSegment(LeftPoint, RightPoint);
#endif

	if (FMathExt::IsInsideTriangleLocal2D(LeftPoint, RightPoint, Loc2D))
	{
		// Location is inside triangle formed by the (LeftPoint, RightPoint) polygon side and origin
		return Location;
	}

	// Helper data struct for checking closest points to lines before LeftPoint and after RightPoint
	struct CheckDataType
	{
		int32 Idx; // Index of the the polygon line begin point
		int32 (TCyclicArray<FVector2D>::*GetNextIdx)(int32); // Method, which returns the index of the polygon line end point
		float NextPossibleClosestDistSqr; // Distance squared to the best possible closest point, which can potentially be generated by the line
		bool bCheckNext; // Is it necessary to check next polygon line
		CheckDataType& OtherData; // Reference to the other direction's data
	}
	CheckData[] =
	{
		// Data for checks of points to the right direction
		{ LeftIdx, &TCyclicArray<FVector2D>::Next, 0.f, true, CheckData[1] },
		// Data for checks of points to the left direction
		{ LeftIdx, &TCyclicArray<FVector2D>::Prev, 0.f, true, CheckData[0] }
	};

	FVector2D ClosestPoint;
	float ClosestPointDistSqr = MAX_FLT;

	// Function finds a better closest point on the next polygon line if applicable
	auto FindClosestPoint = [this, &PointsC, &Loc2D, &ClosestPoint, &ClosestPointDistSqr](CheckDataType &Data)
	{
		if (!Data.bCheckNext) return;

		int32 NextIdx = (PointsC.*Data.GetNextIdx)(Data.Idx); // Get line end point index
		const FVector2D &LineBegin = Points[Data.Idx];
		const FVector2D &LineEnd = Points[NextIdx];

#if WITH_EDITOR
		DrawDebugSide(LineBegin, LineEnd);
#endif

		FVector2D LineClosestPoint = FMath::ClosestPointOnSegment2D(Loc2D, LineBegin, LineEnd);
		float LineClosestPointDistSqr = (LineClosestPoint - Loc2D).SizeSquared();

		if (LineClosestPointDistSqr < ClosestPointDistSqr)
		{
			// Closest point to the checked line is better than the current closest point
			ClosestPoint = LineClosestPoint;
			ClosestPointDistSqr = LineClosestPointDistSqr;

			if (Data.OtherData.bCheckNext)
			{
				// Closest point has changed: recheck if it makes sence to check the other direction
				Data.OtherData.bCheckNext = ClosestPointDistSqr > Data.OtherData.NextPossibleClosestDistSqr;
			}
		}

		if (UNLIKELY(NextIdx == Data.OtherData.Idx))
		{
			// All points are checked
			Data.bCheckNext = false;
			Data.OtherData.bCheckNext = false;
			return;
		}

		// Check if it makes sence to check the next line for the current direction
		Data.bCheckNext = IsNeedToCheckNextLine(Loc2D, LineEnd, ClosestPointDistSqr, OUT Data.NextPossibleClosestDistSqr);

		Data.Idx = NextIdx;
	};

	// Check lines to the right and left from the point in turns
	for (uint32 i = 0; CheckData[0].bCheckNext || CheckData[1].bCheckNext; i ^= 1u)
	{
		FindClosestPoint(CheckData[i]);
	}

	return FVector(ClosestPoint, Location.Z);
}

int32 UPolygonArea2DComponent::FindContainingSector(const FVector2D& Location)
{
	check(Points.Num() > 2);

	// Predicates return if a point is in the left/right half-plane relative to the vector from the origin to the Location
	auto IsLeft = [Location](const FVector2D& Point) { return (Point ^ Location) >= 0.f; };
	auto IsRight = [Location](const FVector2D& Point) { return (Point ^ Location) < 0.f; };

	int32 AnyLeftIndex;
	int32 AnyRightIndex;
	bool LastIsRight = IsRight(Points.Last());
	if (LastIsRight) // Left points form one continuous sequence in the points array
	{
		AnyLeftIndex = FindAnyPoint(Points, IsLeft);
		AnyRightIndex = Points.Num() - 1;

		checkf(AnyLeftIndex != INDEX_NONE, TEXT("Points array is invalid: all points are to the right from the point"));
	}
	else // Right points form one continuous sequence in the points array
	{
		bool FirstIsLeft = IsLeft(Points[0]);
		if (FirstIsLeft)
		{
			AnyLeftIndex = 0;
			AnyRightIndex = FindAnyPoint(Points, IsRight);

			checkf(AnyRightIndex != INDEX_NONE, TEXT("Points array is invalid: all points are to the left from the point"));
		}
		else
		{
			// The last point is in the left half-plane, the first point is in the right,
			// so the last and the first points form the containing sector
			return Points.Num() - 1;
		}
	}

	// AnyLeftIndex belongs to IsLeft continuous point sequences
	// AnyRightIndex belongs to IsRight continuous point sequences
	// Those two point sequences are adjacent, so the searched sector is the partition point between them
	auto BeginIt = Points.GetData();
	auto PartitionIt = std::partition_point(BeginIt + AnyLeftIndex, BeginIt + AnyRightIndex, IsLeft);
	return (PartitionIt - BeginIt) - 1;
}

#if WITH_EDITOR
void UPolygonArea2DComponent::DrawDebugSegment(const FVector2D& A, const FVector2D& B)
{
	using namespace Utils;

	if (bDrawTestedSegments)
	{
		FVector ActorLocation = GetOwner()->GetActorLocation();

		DrawDebugLine(GetWorld(),
			ActorLocation, ActorLocation + To3D(A),
			FColor::Cyan, false, -1., (uint8)1u, 10.f);

		DrawDebugLine(GetWorld(),
			ActorLocation + To3D(A), ActorLocation + To3D(B),
			FColor::Cyan, false, -1., (uint8)1u, 10.f);

		DrawDebugLine(GetWorld(),
			ActorLocation + To3D(B), ActorLocation,
			FColor::Cyan, false, -1., (uint8)1u, 10.f);
	}
}

void UPolygonArea2DComponent::DrawDebugSide(const FVector2D& A, const FVector2D& B)
{
	using namespace Utils;

	if (bDrawTestedSegments)
	{
		FVector ActorLocation = GetOwner()->GetActorLocation();

		DrawDebugLine(GetWorld(),
			ActorLocation + To3D(A), ActorLocation + To3D(B),
			FColor::Green, false, -1., (uint8)1u, 30.f);
	}
}
#endif