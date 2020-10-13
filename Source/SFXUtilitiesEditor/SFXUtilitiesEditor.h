// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IListenedModuleInterface.h"
#include "Modules/ModuleManager.h"

class FSFXUtilitiesEditor : public IListenedModuleInterface
{
public:
	// Begin IListenedModuleInterface implementation
	void StartupModule() override;
	void ShutdownModule() override;
	// End IListenedModuleInterface implementation
};