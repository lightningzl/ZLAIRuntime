#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ZLAIServiceTypes.h"
#include "ZLSocialObservation.h"
#include "ZLSocialToolRegistry.h"
#include "ZLSocialSandboxGameMode.generated.h"

class AZLSocialSandboxNpc;

struct FZLSocialSandboxDecisionDebug
{
	FString RequestId;
	FString Provider;
	FString Intent;
	FString ToolName;
	FName ToolResult;
	int64 StateVersion = 0;
	int32 LatencyMs = 0;
	bool bSpeechAccepted = false;
	bool bInFlight = false;
};

UCLASS()
class ZL_API AZLSocialSandboxGameMode final : public AGameModeBase
{
	GENERATED_BODY()

public:
	AZLSocialSandboxGameMode();

	UFUNCTION(Exec)
	void ResetSocialSandbox();

	UFUNCTION(Exec)
	void RunSocialSandboxDemo();

	const TArray<TObjectPtr<AZLSocialSandboxNpc>>& GetSandboxNpcs() const { return SandboxNpcs; }
	AZLSocialSandboxNpc* FindSandboxNpc(FName StableId) const;
	FText SubmitSpeech(FName SpeechMode, FName TargetId, const FString& Text);
	FText SubmitAction(FName TargetId, const FString& Text);
	const FZLSocialObservationSettings& GetObservationSettings() const { return ObservationSettings; }
	const FZLSocialSandboxDecisionDebug& GetDecisionDebug() const { return DecisionDebug; }
	FText BuildInspectorText(FName NpcId) const;

protected:
	virtual void BeginPlay() override;

private:
	void SpawnEnvironment();
	void SpawnNpc(FName StableId, const TCHAR* DisplayName, const FVector& Location, const FRotator& Rotation);
	void DispatchActionObservation(EZLSocialActionType Action, EZLSocialActionPhase Phase, FName TargetId);
	void DispatchNpcActionObservation(AZLSocialSandboxNpc* Actor, EZLSocialActionType Action, EZLSocialActionPhase Phase, FName TargetId);
	void RequestGuardDecision(AZLSocialSandboxNpc* Guard, const FZLSocialObservation& Trigger, const FString& SpeechContent);
	void HandleGuardDecision(AZLSocialSandboxNpc* Guard, const FZLDecisionResponse& Response, double SentAtSeconds);
	void HandleGuardDecisionFailure(AZLSocialSandboxNpc* Guard, const FZLServiceError& Error, double SentAtSeconds);
	void ExecuteGuardTool(AZLSocialSandboxNpc* Guard, const FZLDecisionResponse& Response);
	void FinishDecisionSmokeTest();
	void RefreshInspector() const;

	UPROPERTY()
	TArray<TObjectPtr<AZLSocialSandboxNpc>> SandboxNpcs;

	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Perception")
	FZLSocialObservationSettings ObservationSettings;
	FZLSocialToolRegistry ToolRegistry;
	FZLSocialSandboxDecisionDebug DecisionDebug;
	TArray<double> GuardExecutionTimes;
	int32 GuardRequestGeneration = 0;
	FTimerHandle DemoTimer;
	FTimerHandle DecisionSmokeTimer;
	FString ExpectedSmokeProvider;
	bool bExpectStaleSmoke = false;
	FVector SmokeInitialGuardLocation = FVector::ZeroVector;
	int64 SmokeInitialGuardVersion = 0;
};
