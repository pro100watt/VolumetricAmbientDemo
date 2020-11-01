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
	void SetListener(const APlayerController* NewListener) { Listener = NewListener; }

protected:
	void BeginPlay() override;

private:
	void UpdateEmitterPosition();

	/** Returns false if failed to update max radius */
	bool UpdateMaxRadius();

	/** Returns false if listener location has not changed */
	bool UpdateListenerLocation();

	UPROPERTY(VisibleAnywhere)
	UPolygonArea2DComponent* Area;

	const APlayerController* Listener;

	FVector ListenerLocation;
	float MaxRadius;
};
