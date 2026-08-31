#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ZLSocialSandboxGameMode.generated.h"

class AZLSocialSandboxNpc;

UCLASS()
class ZL_API AZLSocialSandboxGameMode final : public AGameModeBase
{
	GENERATED_BODY()

public:
	AZLSocialSandboxGameMode();

	UFUNCTION(Exec)
	void ResetSocialSandbox();

	const TArray<TObjectPtr<AZLSocialSandboxNpc>>& GetSandboxNpcs() const { return SandboxNpcs; }
	AZLSocialSandboxNpc* FindSandboxNpc(FName StableId) const;

protected:
	virtual void BeginPlay() override;

private:
	void SpawnEnvironment();
	void SpawnNpc(FName StableId, const TCHAR* DisplayName, const FVector& Location, const FRotator& Rotation);

	UPROPERTY()
	TArray<TObjectPtr<AZLSocialSandboxNpc>> SandboxNpcs;
};
