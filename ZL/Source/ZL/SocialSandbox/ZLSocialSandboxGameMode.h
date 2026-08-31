#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ZLSocialObservation.h"
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
	FText SubmitSpeech(FName SpeechMode, FName TargetId, const FString& Text);
	FText SubmitAction(FName TargetId, const FString& Text);
	const FZLSocialObservationSettings& GetObservationSettings() const { return ObservationSettings; }
	FText BuildInspectorText(FName NpcId) const;

protected:
	virtual void BeginPlay() override;

private:
	void SpawnEnvironment();
	void SpawnNpc(FName StableId, const TCHAR* DisplayName, const FVector& Location, const FRotator& Rotation);
	void DispatchActionObservation(EZLSocialActionType Action, EZLSocialActionPhase Phase, FName TargetId);
	void RefreshInspector() const;

	UPROPERTY()
	TArray<TObjectPtr<AZLSocialSandboxNpc>> SandboxNpcs;

	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Perception")
	FZLSocialObservationSettings ObservationSettings;
};
