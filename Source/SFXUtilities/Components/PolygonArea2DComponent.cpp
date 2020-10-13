// Fill out your copyright notice in the Description page of Project Settings.


#include "PolygonArea2DComponent.h"

#include "SFXUtilities/Utilities/ArrayUtils.h"
#include "SFXUtilities/Utilities/VectorUtils.h"

#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif

namespace
{
	template<class Pred>
	int32 FindAnyPoint(const TArray<FVector2D>& Points, Pred IsOK)
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

		return N - 1;
	}

	bool IsNeedToCheckNextPoint(const FVector2D& Location, const FVector2D& PointVector, float ClosestPointDistSqr, float& OutBestPossibleClosestDistSqr)
	{
		check(!PointVector.IsNearlyZero());

		const float Dot1 = Location | PointVector;
		// See if closest point is in center
		if (Dot1 <= 0) return false;

		// See if location projection is less than point to origin length,
		// which means that the distance to any current point's line is closer than any next point can produce
		const float Dot2 = PointVector | PointVector;
		if (Dot1 < Dot2) return false;

		// Calculate the best closest distance, which may be produced by next point
		OutBestPossibleClosestDistSqr = (PointVector * (Dot1 / Dot2) - Location).SizeSquared();

		// If the best closest distance is not better than current closest distance
		// we don't need to check further
		if (ClosestPointDistSqr <= OutBestPossibleClosestDistSqr) return false;

		return true;
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

FVector UPolygonArea2DComponent::GetClosestPoint(const FVector& Location)
{
	using namespace Utils;

	const FVector2D &Loc2D = As2D(Location);

	if (MinBox.IsInside(Loc2D)) return Location;

	auto PointsC = GetCyclic(Points);

	int32 LeftIdx = FindContainingSector(Loc2D);
	int32 RightIdx = PointsC.Next(LeftIdx);

	const FVector2D &LeftPoint = Points[LeftIdx];
	const FVector2D &RightPoint = Points[RightIdx];

#if WITH_EDITOR
	if (bDrawTestedSegments)
	{
		FVector ActorLocation = GetOwner()->GetActorLocation();

		DrawDebugLine(GetWorld(),
			ActorLocation,
			ActorLocation + To3D(LeftPoint),
			FColor::Cyan, false, -1., (uint8)1u, 10.f);

		DrawDebugLine(GetWorld(),
			ActorLocation + To3D(LeftPoint),
			ActorLocation + To3D(RightPoint),
			FColor::Cyan, false, -1., (uint8)1u, 10.f);

		DrawDebugLine(GetWorld(),
			ActorLocation + To3D(RightPoint),
			ActorLocation,
			FColor::Cyan, false, -1., (uint8)1u, 10.f);
	}
#endif

	// Check if inside segment triangle
	{
		float LCross = LeftPoint ^ Loc2D;
		float RCross = Loc2D ^ RightPoint;

		ensure(LCross >= 0.f && RCross >= 0.f);

		float InvHalfArea = 1.f / (LeftPoint ^ RightPoint);
		float D = 1.f - InvHalfArea * (LCross + RCross);

		if (D >= 0.f) return Location;
	}

	// Find closest point
	{
		FVector2D ClosestPoint = FMath::ClosestPointOnSegment2D(Loc2D, LeftPoint, RightPoint);
		float ClosestPointDistSqr = (ClosestPoint - Loc2D).SizeSquared();

		struct CheckDataType
		{
			int32 Idx;
			int32 (TCyclicArray<FVector2D>::*GetNextIdx)(int32);
			float NextPossibleClosestDistSqr;
			bool bCheckNext;
			float& OtherNextPossibleClosestDistSqr;
			bool& bOtherCheckNext;

		} CheckData[] =
		{
			{ LeftIdx, &TCyclicArray<FVector2D>::Prev, 0.f, true, CheckData[1].NextPossibleClosestDistSqr, CheckData[1].bCheckNext},
			{ RightIdx, &TCyclicArray<FVector2D>::Next, 0.f, true, CheckData[0].NextPossibleClosestDistSqr, CheckData[0].bCheckNext},
		};

		CheckData[0].bCheckNext = IsNeedToCheckNextPoint(Loc2D, LeftPoint, ClosestPointDistSqr, CheckData[0].NextPossibleClosestDistSqr);
		CheckData[1].bCheckNext = IsNeedToCheckNextPoint(Loc2D, RightPoint, ClosestPointDistSqr, CheckData[1].NextPossibleClosestDistSqr);

		auto CheckNextSegment = [this, &PointsC, &Loc2D, &ClosestPoint, &ClosestPointDistSqr](CheckDataType &Data)
		{
			if (!Data.bCheckNext) return;

			auto GetNextIdx = Data.GetNextIdx;
			int32 NextIdx = (PointsC.*GetNextIdx)(Data.Idx);
			const FVector2D &OldPoint = Points[Data.Idx];
			const FVector2D &NewPoint = Points[NextIdx];
			FVector2D PotentialClosestPoint = FMath::ClosestPointOnSegment2D(Loc2D, OldPoint, NewPoint);

#if WITH_EDITOR
			if (bDrawTestedSegments)
			{
				FVector ActorLocation = GetOwner()->GetActorLocation();

				DrawDebugLine(GetWorld(),
					ActorLocation + To3D(OldPoint),
					ActorLocation + To3D(NewPoint),
					FColor::Green, false, -1., (uint8)1u, 30.f);
			}
#endif

			float PotentialClosestPointDistSqr = (PotentialClosestPoint - Loc2D).SizeSquared();
			if (PotentialClosestPointDistSqr < ClosestPointDistSqr)
			{
				ClosestPoint = PotentialClosestPoint;
				ClosestPointDistSqr = PotentialClosestPointDistSqr;

				if (Data.bOtherCheckNext) Data.bOtherCheckNext = ClosestPointDistSqr > Data.OtherNextPossibleClosestDistSqr;
			}
			Data.bCheckNext = IsNeedToCheckNextPoint(Loc2D, NewPoint, ClosestPointDistSqr, OUT Data.NextPossibleClosestDistSqr);

			Data.Idx = NextIdx;
		};

		for (uint32 i = 0; (CheckData[0].Idx != CheckData[1].Idx) && (CheckData[0].bCheckNext || CheckData[1].bCheckNext); i ^= 1u)
		{
			CheckNextSegment(CheckData[i]);
		}

		return FVector(ClosestPoint, Location.Z);
	}
}

int32 UPolygonArea2DComponent::FindContainingSector(const FVector2D& Point)
{
	check(Points.Num() > 1);

	auto IsLeft = [Point](const FVector2D& V) { return (V ^ Point) >= 0.f; };
	auto IsRight = [Point](const FVector2D& V) { return (V ^ Point) < 0.f; };

	int32 LeftIndex;
	int32 RightIndex;
	bool LastIsRight = IsRight(Points.Last());
	if (LastIsRight)
	{
		LeftIndex = FindAnyPoint(Points, IsLeft);
		RightIndex = Points.Num() - 1;
	}
	else
	{
		bool FirstIsLeft = IsLeft(Points[0]);
		if (FirstIsLeft)
		{
			LeftIndex = 0;
			RightIndex = FindAnyPoint(Points, IsRight);
		}
		else
		{
			return Points.Num() - 1;
		}
	}

	auto BeginIt = Points.GetData();
	auto PartitionIt = std::partition_point(BeginIt + LeftIndex, BeginIt + RightIndex, IsLeft);
	return (PartitionIt - BeginIt) - 1;
}