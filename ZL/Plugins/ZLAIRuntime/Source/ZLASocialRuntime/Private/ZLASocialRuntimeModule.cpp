#include "ZLASocialRuntimeModule.h"

#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogZLASocialRuntime, Log, All);

void FZLASocialRuntimeModule::StartupModule()
{
	UE_LOG(LogZLASocialRuntime, Log, TEXT("ZLASocialRuntime started"));
}

void FZLASocialRuntimeModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FZLASocialRuntimeModule, ZLASocialRuntime)
