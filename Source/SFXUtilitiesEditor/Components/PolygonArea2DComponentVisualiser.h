#pragma once

#include "ComponentVisualizer.h"

class UPolygonArea2DComponent;

//Base class for clickable targeting editing proxies
struct HAmbientAreaVisProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();

	HAmbientAreaVisProxy(const UActorComponent* InComponent)
		: HComponentVisProxy(InComponent, HPP_Wireframe)
	{}
};

//Proxy for points
struct HPointProxy : public HAmbientAreaVisProxy
{
	DECLARE_HIT_PROXY();

	HPointProxy(const UActorComponent* InComponent, int32 InPointIndex)
		: HAmbientAreaVisProxy(InComponent), PointIndex(InPointIndex)
	{}

	int32 PointIndex;
};

//Proxy for lines
struct HLineProxy : public HAmbientAreaVisProxy
{
	DECLARE_HIT_PROXY();

	HLineProxy(const UActorComponent* InComponent, int32 InBeginPointIndex)
		: HAmbientAreaVisProxy(InComponent), BeginPointIndex(InBeginPointIndex)
	{}

	int32 BeginPointIndex;
};

class SFXUTILITIESEDITOR_API FPolygonArea2DComponentVisualiser : public FComponentVisualizer
{
public:
	FPolygonArea2DComponentVisualiser();
	virtual ~FPolygonArea2DComponentVisualiser();

	// Begin FComponentVisualizer interface
	void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
	bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;
	bool HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
	// End FComponentVisualizer interface

private:
	// Begin Delegates
	void AddDelegates();
	void RemoveDelegates();

	void OnNewActorsDropped(const TArray<UObject*> &Objects, const TArray<AActor*> &Actors);
	FDelegateHandle OnNewActorsDroppedHandle;
	// End Delegates

	// Begin Helpers
	UPolygonArea2DComponent* GetAmbientAreaComponent() const;

	bool CanDeletePoint(int32 PointIndex);
	void DrawSegment(const UPolygonArea2DComponent *AreaComp, FPrimitiveDrawInterface* PDI, int32 BeginIndex, int32 EndIndex);
	void UpdateBoxes(int32 Index = INDEX_NONE);
	void Constrain(int32 Index, FVector2D& Delta);
	void ConstrainByHalfPlane(FVector2D& V, const FVector2D& VecCCW, const FVector2D& VecCW);
	void ConstrainByRadius(FVector2D& V, const FVector2D& VAdj, const float PlainSig);
	// End Helpers

	FComponentPropertyPath ComponentPropertyPath;

	int32 SelectedPoint;
	int32 SelectedLineBegin;
};