#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "ZLSocialPersonaSettings.generated.h"

class UDataRegistry;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "ZL Social Persona"))
class ZLASOCIALRUNTIME_API UZLSocialPersonaSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Only these registries may be queried by Persona tooling or runtime spawners.
	UPROPERTY(Config, EditAnywhere, Category = "Persona Registry")
	TArray<TSoftObjectPtr<UDataRegistry>> PersonaRegistries;

	bool IsConfiguredRegistry(const UDataRegistry* Registry) const;
};
