// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Math/Box.h"

#include "PolygonArea2DComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SFXUTILITIES_API UPolygonArea2DComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPolygonArea2DComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Returns true if the Location is within Radius from the MaxBox bounding box in 2D */
	bool IsWithinRadius(const FVector& Location, float Radius);

	/** Returns the closest to the Location point inside the polygon in 2D (Z is copied from the Location) */
	FVector FindClosestPoint(const FVector &Location);

private:
	/**
	 * Finds two adjacent points, which form a sector from the origin, which contains the Location
	 * Returns the index of the first point
	 */
	int32 FindContainingSector(const FVector2D& Location);

	UPROPERTY()
	TArray<FVector2D> Points;
	UPROPERTY()
	FBox2D MinBox;
	UPROPERTY()
	FBox2D MaxBox;


#if WITH_EDITOR
	void DrawDebugSegment(const FVector2D& A, const FVector2D& B);
	void DrawDebugSide(const FVector2D& A, const FVector2D& B);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Editor, meta = (AllowPrivateAccess = "true"))
	FLinearColor EditorSelectedColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Editor, meta = (AllowPrivateAccess = "true"))
	FLinearColor EditorUnselectedColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Editor, meta = (AllowPrivateAccess = "true"))
	FLinearColor EditorBoxColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Editor, meta = (AllowPrivateAccess = "true", ClampMin = "10.0"))
	float MinRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editor|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawArea;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editor|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawBoxes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editor|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawTestedSegments;

	friend class FPolygonArea2DComponentVisualiser;
#endif
};
