#pragma once

#include "Modules/ModuleManager.h"

class IModuleListenerInterface
{
public:
	virtual void OnStartupModule() {};
	virtual void OnShutdownModule() {};
};

class IListenedModuleInterface : public IModuleInterface
{
public:
	void StartupModule() override
	{
		if (!IsRunningCommandlet())
		{
			AddModuleListeners();
			for (const auto &Listener : ModuleListeners)
			{
				Listener->OnStartupModule();
			}
		}
	}

	void ShutdownModule() override
	{
		for (const auto& Listener : ModuleListeners)
		{
			Listener->OnShutdownModule();
		}
	}

	virtual void AddModuleListeners() {};

protected:
	TArray<TSharedRef<IModuleListenerInterface>> ModuleListeners;
};