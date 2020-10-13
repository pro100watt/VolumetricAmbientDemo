// Copyright Epic Games, Inc. All Rights Reserved.

#include "PolygonArea2DComponentVisualiser.h"

#include "SceneManagement.h"

#include "SFXUtilities/Components/PolygonArea2DComponent.h"

#include "SFXUtilitiesEditor/Utilities/FBoxUtils.h"
#include "SFXUtilities/Utilities/VectorUtils.h"
#include "SFXUtilities/Utilities/ArrayUtils.h"

IMPLEMENT_HIT_PROXY(HAmbientAreaVisProxy, HComponentVisProxy);
IMPLEMENT_HIT_PROXY(HPointProxy, HAmbientAreaVisProxy);
IMPLEMENT_HIT_PROXY(HLineProxy, HAmbientAreaVisProxy)

FPolygonArea2DComponentVisualiser::FPolygonArea2DComponentVisualiser()
	: SelectedPoint(INDEX_NONE)
	, SelectedLineBegin(INDEX_NONE)
{
	AddDelegates();
}

FPolygonArea2DComponentVisualiser::~FPolygonArea2DComponentVisualiser()
{
	RemoveDelegates();
}

void FPolygonArea2DComponentVisualiser::AddDelegates()
{
	OnNewActorsDroppedHandle = FEditorDelegates::OnNewActorsDropped.AddRaw(this, &FPolygonArea2DComponentVisualiser::OnNewActorsDropped);
}

void FPolygonArea2DComponentVisualiser::RemoveDelegates()
{
	FEditorDelegates::OnNewActorsDropped.Remove(OnNewActorsDroppedHandle);
}

void FPolygonArea2DComponentVisualiser::OnNewActorsDropped(const TArray<UObject*>& Objects, const TArray<AActor*>& Actors)
{
	// TODO: Not implemented yet
}

void FPolygonArea2DComponentVisualiser::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	if(const UPolygonArea2DComponent *AreaComponent = Cast<const UPolygonArea2DComponent>(Component))
	{
		const auto& Points = AreaComponent->Points;
		if (Points.Num() < 3) return;

		FMatrix TransformMatrix = AreaComponent->GetOwner()->GetActorTransform().ToMatrixNoScale();
		FBox MinBox(FVector(AreaComponent->MinBox.Min, -200.f), FVector(AreaComponent->MinBox.Max, 200.f));
		FBox MaxBox(FVector(AreaComponent->MaxBox.Min, -200.f), FVector(AreaComponent->MaxBox.Max, 200.f));
		DrawWireBox(PDI, TransformMatrix, MinBox, AreaComponent->EditorBoxColor, SDPG_World, 2.f);
		DrawWireBox(PDI, TransformMatrix, MaxBox, AreaComponent->EditorBoxColor, SDPG_World, 2.f);

		for (int32 i = 0; i < Points.Num() - 1; i++)
		{
			DrawSegment(AreaComponent, PDI, i, i + 1);
		}
		{
			DrawSegment(AreaComponent, PDI, Points.Num() - 1, 0);
		}
	}
}

void FPolygonArea2DComponentVisualiser::DrawSegment(const UPolygonArea2DComponent* AreaComp, FPrimitiveDrawInterface* PDI, int32 BeginIndex, int32 EndIndex)
{
	using namespace Utils;

	auto& Points = AreaComp->Points;
	const FLinearColor SelectedColor = AreaComp->EditorSelectedColor;
	const FLinearColor UnselectedColor = AreaComp->EditorUnselectedColor;

	FLinearColor PointColor = (BeginIndex == SelectedPoint) ? SelectedColor : UnselectedColor;

	FVector BeginPointLocal = To3D(Points[BeginIndex]);
	FVector EndPointLocal = To3D(Points[EndIndex]);

	FVector BeginPoint(AreaComp->GetOwner()->GetActorLocation() + BeginPointLocal);
	FVector EndPoint(AreaComp->GetOwner()->GetActorLocation() + EndPointLocal);

	PDI->SetHitProxy(new HPointProxy(AreaComp, BeginIndex));
	PDI->DrawPoint(BeginPoint, PointColor, 20.f, SDPG_World);
	PDI->SetHitProxy(nullptr);

	FLinearColor LineColor = (BeginIndex == SelectedLineBegin) ? SelectedColor : UnselectedColor;

	PDI->SetHitProxy(new HLineProxy(AreaComp, BeginIndex));
	PDI->DrawLine(BeginPoint, EndPoint, LineColor, SDPG_World, 3.0f);
	PDI->SetHitProxy(nullptr);
}

bool FPolygonArea2DComponentVisualiser::VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	bool bEditing = false;

	ComponentPropertyPath.Reset();
	SelectedPoint = INDEX_NONE;
	SelectedLineBegin = INDEX_NONE;

	if (VisProxy && VisProxy->Component.IsValid())
	{
		const UPolygonArea2DComponent* Comp = CastChecked<const UPolygonArea2DComponent>(VisProxy->Component.Get());
		ComponentPropertyPath = FComponentPropertyPath(Comp);
	}

	if (ComponentPropertyPath.IsValid())
	{
		bEditing = true;

		if (VisProxy->IsA(HPointProxy::StaticGetType()))
		{
			HPointProxy* Proxy = static_cast<HPointProxy*>(VisProxy);
			SelectedPoint = Proxy->PointIndex;

			UE_LOG(LogTemp, Warning, TEXT("HPointProxy(%i) [%s]"), SelectedPoint, *GetAmbientAreaComponent()->Points[SelectedPoint].ToString());

			{
				FString Result = CanDeletePoint(SelectedPoint) ? "Can delete" : "Not deletable";
				UE_LOG(LogTemp, Warning, TEXT("%s"), *Result);
			}
		}
		else if (VisProxy->IsA(HLineProxy::StaticGetType()))
		{
			HLineProxy* Proxy = static_cast<HLineProxy*>(VisProxy);
			SelectedLineBegin = Proxy->BeginPointIndex;

			UE_LOG(LogTemp, Warning, TEXT("HLineProxy(%i)"), SelectedLineBegin);
		}
	}

	return bEditing;
}


bool FPolygonArea2DComponentVisualiser::GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocaction) const
{
	bool bHasLocation = false;

	auto TargetComponent = GetAmbientAreaComponent();
	if (TargetComponent)
	{
		using namespace Utils;

		FVector OwnerLocation = TargetComponent->GetOwner()->GetActorLocation();

		const auto& Points = TargetComponent->Points;
		if (Points.IsValidIndex(SelectedPoint))
		{
			OutLocaction = OwnerLocation + To3D(Points[SelectedPoint]);

			bHasLocation = true;
		}
		else if (Points.IsValidIndex(SelectedLineBegin))
		{
			int32 SelectedLineEnd = GetCyclic(Points).Next(SelectedLineBegin);
			FVector2D MidPoint = 0.5f * (Points[SelectedLineBegin] + Points[SelectedLineEnd]);
			OutLocaction = OwnerLocation + To3D(MidPoint);

			bHasLocation = true;
		}
	}

	return bHasLocation;
}

bool FPolygonArea2DComponentVisualiser::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	bool bHandled = false;

	auto TargetComponent = GetAmbientAreaComponent();
	if (TargetComponent)
	{
		using namespace Utils;

		DeltaTranslate.Z = 0.f;

		FVector2D &Delta2D = As2D(DeltaTranslate);

		auto& Points = TargetComponent->Points;
		if (Points.IsValidIndex(SelectedPoint))
		{
			Constrain(SelectedPoint, Delta2D);

			Points[SelectedPoint] += Delta2D;

			UpdateBoxes(SelectedPoint);

			bHandled = true;
		}
		else if (Points.IsValidIndex(SelectedLineBegin))
		{
			int32 SelectedLineEnd = GetCyclic(Points).Next(SelectedLineBegin);

			Constrain(SelectedLineBegin, Delta2D);
			Constrain(SelectedLineEnd, Delta2D);

			Points[SelectedLineBegin] += Delta2D;
			Points[SelectedLineEnd] += Delta2D;

			UpdateBoxes(SelectedLineBegin);
			UpdateBoxes(SelectedLineEnd);

			bHandled = true;
		}
	}

	return bHandled;
}

bool FPolygonArea2DComponentVisualiser::HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	auto TargetComponent = GetAmbientAreaComponent();
	if (TargetComponent == nullptr)
	{
		return false;
	}

	bool bHandled = false;

	auto& Points = TargetComponent->Points;
	if (IE_Pressed == Event && EKeys::Delete == Key)
	{
		if (Points.IsValidIndex(SelectedPoint))
		{
			if (CanDeletePoint(SelectedPoint))
			{
				TargetComponent->Points.RemoveAt(SelectedPoint);
				SelectedPoint = INDEX_NONE;

				UpdateBoxes();
			}

			bHandled = true;
		}
	}
	else if (IE_DoubleClick == Event && EKeys::LeftMouseButton == Key)
	{
		if (Points.IsValidIndex(SelectedLineBegin))
		{
			SelectedPoint = SelectedLineBegin + 1;

			int32 SelectedLineEnd = Utils::GetCyclic(Points).Next(SelectedLineBegin);
			FVector2D NewPoint = 0.5f * (Points[SelectedLineBegin] + Points[SelectedLineEnd]);

			TargetComponent->Points.Insert(NewPoint, SelectedPoint);
			SelectedLineBegin = INDEX_NONE;

			bHandled = true;
		}
	}

	return bHandled;
}

UPolygonArea2DComponent* FPolygonArea2DComponentVisualiser::GetAmbientAreaComponent() const
{
	if (!ComponentPropertyPath.IsValid())
	{
		return nullptr;
	}

	return Cast<UPolygonArea2DComponent>(ComponentPropertyPath.GetComponent());
}

void FPolygonArea2DComponentVisualiser::UpdateBoxes(int32 Index)
{
	auto TargetComponent = GetAmbientAreaComponent();
	if (TargetComponent == nullptr)
	{
		return;
	}

	const auto& Points = TargetComponent->Points;

	bool bRebuildMax = true;
	if (Index != INDEX_NONE)
	{
		const FVector2D &Point = Points[Index];
		FBox2D& MaxBox = TargetComponent->MaxBox;
		FBox2D OldMaxBox = MaxBox;

		MaxBox += Point;

		bRebuildMax = (OldMaxBox == MaxBox);
	}

	if (bRebuildMax)
	{
		FBox2D& MaxBox = TargetComponent->MaxBox;
		MaxBox = FBox2D(Points.GetData(), Points.Num());
	}

	FBox2D& MinBox = TargetComponent->MinBox;
	MinBox = TargetComponent->MaxBox;

	Utils::Inscribe(MinBox, Points.GetData(), Points.Num());
}

void FPolygonArea2DComponentVisualiser::Constrain(int32 Index, FVector2D& Delta)
{
	auto TargetComponent = GetAmbientAreaComponent();
	if (TargetComponent == nullptr)
	{
		return;
	}

	const float MIN_RADIUS = TargetComponent->MinRadius;
	const float MIN_R_SQR = MIN_RADIUS * MIN_RADIUS;
	auto& Points = TargetComponent->Points;

	auto PointsC = Utils::GetCyclic(Points);
	int32 CCWIndex = PointsC.Prev(Index);
	int32 CWIndex = PointsC.Next(Index);

	FVector2D VecCCW = Points[CCWIndex];
	FVector2D V = Points[Index] + Delta;
	FVector2D VecCW = Points[CWIndex];

	{
		ConstrainByHalfPlane(V, VecCCW, VecCW);
		ConstrainByRadius(V, VecCCW, 1.f);
		ConstrainByRadius(V, VecCW, -1.f);
	}

	Delta = V - Points[Index];
}

bool FPolygonArea2DComponentVisualiser::CanDeletePoint(int32 PointIndex)
{
	auto TargetComponent = GetAmbientAreaComponent();
	if (TargetComponent == nullptr)
	{
		return false;
	}

	auto& Points = TargetComponent->Points;
	if (Points.Num() <= 4) return false;

	auto PointsC = Utils::GetCyclic(Points);
	int32 CCWIndex = PointsC.Prev(PointIndex);
	int32 CWIndex = PointsC.Next(PointIndex);

	const FVector2D &PointCCW = Points[CCWIndex];
	const FVector2D &PointCW = Points[CWIndex];

	const float MIN_RADIUS = GetAmbientAreaComponent()->MinRadius;

	FVector2D Line = PointCCW - PointCW;
	FVector2D Norm = PointCW - Line * ((PointCW | Line) / (Line | Line));

	return ((Line ^ PointCW) > 0.f) && (Norm.SizeSquared() > MIN_RADIUS * MIN_RADIUS);
}

void FPolygonArea2DComponentVisualiser::ConstrainByHalfPlane(FVector2D& V, const FVector2D& VecCCW, const FVector2D& VecCW)
{
	const float MIN_RADIUS = GetAmbientAreaComponent()->MinRadius;
	float SideSign = FMath::Sign(VecCCW ^ VecCW);

	FVector2D Direction = (VecCCW.GetSafeNormal() + VecCW.GetSafeNormal()).GetSafeNormal();

	float ProjSize = V | Direction;
	if (ProjSize * SideSign >= MIN_RADIUS) return;

	V -= (ProjSize - SideSign * MIN_RADIUS) * Direction;
}

void FPolygonArea2DComponentVisualiser::ConstrainByRadius(FVector2D& V, const FVector2D& VAdj, const float PlainSig)
{
	const float MIN_RADIUS = GetAmbientAreaComponent()->MinRadius;
	FVector2D VAdjNorm = VAdj.GetSafeNormal();
	FVector2D SideNorm(-VAdjNorm.Y, VAdjNorm.X);
	FVector2D RadiusOffset = MIN_RADIUS * PlainSig * SideNorm;

	float SideCross = PlainSig * (VAdj ^ (V - RadiusOffset));
	if (SideCross > 0.f) return;

	V += RadiusOffset - (SideNorm | V) * SideNorm;
}