#pragma once

#include "Modules/ModuleManager.h"

class FZLASocialRuntimeModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
