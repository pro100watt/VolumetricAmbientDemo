// Fill out your copyright notice in the Description page of Project Settings.


#include "FMODVolumetricEmitter.h"

#include "SFXUtilities/Components/PolygonArea2DComponent.h"

#include "FMODEvent.h"

#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif

AFMODVolumetricEmitter::AFMODVolumetricEmitter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	AudioComponent->SetupAttachment(RootComponent);

	Area = CreateDefaultSubobject<UPolygonArea2DComponent>(TEXT("Area"));
}

void AFMODVolumetricEmitter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (Listener != nullptr)
	{
		SetListenerPosition(Listener->GetComponentLocation());
#if WITH_EDITOR
		{
			DrawDebugSphere(GetWorld(), AudioComponent->GetComponentLocation(), GetAttenuationRadius(), 20, FColor::Orange);
		}
#endif
	}
}

void AFMODVolumetricEmitter::SetListenerPosition(const FVector& Pos)
{
	FVector LocalPos = Pos - GetActorLocation();
	if (!Area->IsWithinRadius(LocalPos, GetAttenuationRadius())) return;

	FVector ClosestPoint = Area->GetClosestPoint(LocalPos);

	AudioComponent->SetRelativeLocation(ClosestPoint);
}

// Would be much better if cached,
// but FMODComponent API does not have callbacks on event or attenuation radius change
float AFMODVolumetricEmitter::GetAttenuationRadius() const
{
	float Radius = 0.f;

	if (!IsValid(AudioComponent) || !AudioComponent->Event.IsValid()) return Radius;

	FMOD::Studio::EventDescription* EventDesc =
		IFMODStudioModule::Get().GetEventDescription(AudioComponent->Event.Get(), EFMODSystemContext::Auditioning);
	if (EventDesc == nullptr) return Radius;
	
	{
		bool bIs3D = false;
		EventDesc->is3D(&bIs3D);
		if (!bIs3D) return Radius;
	}

	if (AudioComponent->AttenuationDetails.bOverrideAttenuation)
	{
		Radius = AudioComponent->AttenuationDetails.MaximumDistance;
	}
	else
	{
		EventDesc->getMaximumDistance(&Radius);
	}
	Radius = FMODUtils::DistanceToUEScale(Radius);

	return Radius;
}