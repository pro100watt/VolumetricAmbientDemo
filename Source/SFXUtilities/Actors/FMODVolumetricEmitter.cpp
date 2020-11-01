// Fill out your copyright notice in the Description page of Project Settings.


#include "FMODVolumetricEmitter.h"

#include "SFXUtilities/Components/PolygonArea2DComponent.h"

#include "FMODEvent.h"
#include "FMODAudioComponent.h"

#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif

AFMODVolumetricEmitter::AFMODVolumetricEmitter()
	: Listener(nullptr)
	, MaxRadius(0.f)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	AudioComponent->SetupAttachment(RootComponent);

	Area = CreateDefaultSubobject<UPolygonArea2DComponent>(TEXT("Area"));
}

void AFMODVolumetricEmitter::BeginPlay()
{
	Super::BeginPlay();

	verifyf(UpdateMaxRadius(), TEXT("Failed to set MaxRadius in AFMODVolumetricEmitter::BeginPlay"));
}

void AFMODVolumetricEmitter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateEmitterPosition();

#if WITH_EDITOR
	DrawDebugSphere(GetWorld(), AudioComponent->GetComponentLocation(), MaxRadius, 20, FColor::Orange);
#endif

#if DO_CHECK
	// Check if MaxRadius has changed at runtime
	{
		float OldMaxRadius = MaxRadius;
		if (UpdateMaxRadius())
		{
			checkf(OldMaxRadius == MaxRadius, TEXT("AFMODVolumetricEmitter does not support changing AttenuationRadius at runtime."));
		}
	}
#endif
}

void AFMODVolumetricEmitter::UpdateEmitterPosition()
{
	if (!UpdateListenerLocation())
	{
		// Listener location did not check
		return;
	}

	FVector LocalListenerPosition = ListenerLocation - GetActorLocation();
	if (!Area->IsWithinRadius(LocalListenerPosition, MaxRadius))
	{
		// Listener is outside sound attenuation radius
		return;
	}

	FVector ClosestPoint = Area->FindClosestPoint(LocalListenerPosition);

	AudioComponent->SetRelativeLocation(ClosestPoint);
}

bool AFMODVolumetricEmitter::UpdateListenerLocation()
{
	if (Listener == nullptr) return false;

	FVector CurrentLocation;
	FVector ListenerFrontDir, ListenerRightDir;
	Listener->GetAudioListenerPosition(CurrentLocation, ListenerFrontDir, ListenerRightDir);

	if (CurrentLocation.Equals(ListenerLocation)) return false;

	ListenerLocation = CurrentLocation;

	return true;
}

// Would be much better if cached, but UFMODAudioComponent does not have attenuation radius OnChanged callbacks
bool AFMODVolumetricEmitter::UpdateMaxRadius()
{
	if (!IsValid(AudioComponent) || !AudioComponent->Event.IsValid()) return false;

	FMOD::Studio::EventDescription* EventDesc =
		IFMODStudioModule::Get().GetEventDescription(AudioComponent->Event.Get(), EFMODSystemContext::Auditioning);

	if (EventDesc == nullptr) return false;
	
	bool bIs3D = false;
	EventDesc->is3D(&bIs3D);

	if (!bIs3D) return false;

	if (AudioComponent->AttenuationDetails.bOverrideAttenuation)
	{
		MaxRadius = AudioComponent->AttenuationDetails.MaximumDistance;
	}
	else
	{
		EventDesc->getMaximumDistance(&MaxRadius);
	}
	MaxRadius = FMODUtils::DistanceToUEScale(MaxRadius);

	return true;
}