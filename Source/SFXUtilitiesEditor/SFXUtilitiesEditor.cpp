// Copyright Epic Games, Inc. All Rights Reserved.

#include "SFXUtilitiesEditor.h"

#include "UnrealEd.h"

#include "Components/PolygonArea2DComponentVisualiser.h"
#include "SFXUtilities/Components/PolygonArea2DComponent.h"

IMPLEMENT_GAME_MODULE(FSFXUtilitiesEditor, SFXUtilitiesEditor);

void FSFXUtilitiesEditor::StartupModule()
{
	IListenedModuleInterface::StartupModule();

	if (GUnrealEd != nullptr)
	{
		TSharedPtr<FPolygonArea2DComponentVisualiser> Visualizer = MakeShareable(new FPolygonArea2DComponentVisualiser());

		if (Visualizer.IsValid())
		{
			GUnrealEd->RegisterComponentVisualizer(UPolygonArea2DComponent::StaticClass()->GetFName(), Visualizer);
			Visualizer->OnRegister();
		}
	}
}

void FSFXUtilitiesEditor::ShutdownModule()
{
	IListenedModuleInterface::ShutdownModule();

	if (GUnrealEd != nullptr)
	{
		GUnrealEd->UnregisterComponentVisualizer(UPolygonArea2DComponent::StaticClass()->GetFName());
	}
}