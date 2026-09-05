#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZLSocialSandboxNpcSpawner.generated.h"

class AZLSocialSandboxNpc;
class UZLSocialPersonaAsset;

UCLASS()
class ZL_API AZLSocialSandboxNpcSpawner final : public AActor
{
	GENERATED_BODY()

public:
	AZLSocialSandboxNpcSpawner();
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSubclassOf<AZLSocialSandboxNpc> NpcClass;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	TObjectPtr<UZLSocialPersonaAsset> PersonaAsset;

	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (EditCondition = "PersonaAsset == nullptr", GetOptions = "GetConfiguredPersonaIdOptions"))
	FName PersonaId;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	FLinearColor BodyColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = "1.0", ClampMax = "100.0"))
	float InitialHealth = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	bool bSpawnOnBeginPlay = true;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Spawner")
	FString LastSpawnResult;

	UFUNCTION(CallInEditor, Category = "Spawner")
	bool SpawnNpc();

	/** Provides only the stable IDs exposed by configured Persona registries. */
	UFUNCTION()
	TArray<FString> GetConfiguredPersonaIdOptions() const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
