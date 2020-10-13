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

	bool IsWithinRadius(const FVector& Location, float Radius);
	FVector GetClosestPoint(const FVector &Location);

private:
	int32 FindContainingSector(const FVector2D& Point);

	UPROPERTY()
	TArray<FVector2D> Points;
	UPROPERTY()
	FBox2D MinBox;
	UPROPERTY()
	FBox2D MaxBox;


#if WITH_EDITOR
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Editor, meta = (AllowPrivateAccess = "true"))
	FLinearColor EditorSelectedColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Editor, meta = (AllowPrivateAccess = "true"))
	FLinearColor EditorUnselectedColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Editor, meta = (AllowPrivateAccess = "true"))
	FLinearColor EditorBoxColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Editor, meta = (AllowPrivateAccess = "true", ClampMin = "10.0"))
	float MinRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editor,Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawArea;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editor,Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawBoxes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editor,Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawTestedSegments;

	friend class FPolygonArea2DComponentVisualiser;
#endif
};
