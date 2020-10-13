// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FMODAmbientSound.h"
#include "FMODVolumetricEmitter.generated.h"

class UPolygonArea2DComponent;

/**
 * 
 */
UCLASS()
class SFXUTILITIES_API AFMODVolumetricEmitter : public AFMODAmbientSound
{
	GENERATED_BODY()

public:
	AFMODVolumetricEmitter();

	void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable)
	void SetListener(const USceneComponent *Other) { Listener = Other; }

	void SetListenerPosition(const FVector& Pos);

protected:
	float GetAttenuationRadius() const;

private:
	UPROPERTY(VisibleAnywhere)
	UPolygonArea2DComponent* Area;

	const USceneComponent *Listener;
};
