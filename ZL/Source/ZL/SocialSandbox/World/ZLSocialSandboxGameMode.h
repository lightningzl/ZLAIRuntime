#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ZLAIServiceTypes.h"
#include "ZLSocialObservation.h"
#include "ZLSocialToolRegistry.h"
#include "SocialSandbox/Decision/ZLSocialSandboxDecisionContext.h"
#include "SocialSandbox/Decision/ZLSocialSandboxDecisionScheduler.h"
#include "SocialSandbox/Decision/ZLSocialSandboxMultiNpcDecision.h"
#include "SocialSandbox/Domain/ZLSocialSandboxConflictState.h"
#include "SocialSandbox/Domain/ZLSocialSandboxPreset.h"
#include "ZLSocialSandboxGameMode.generated.h"

class AZLSocialSandboxNpc;
class AZLSocialSandboxPawn;
class AController;
struct FZLSocialSandboxDamageResult;
struct FZLSocialSandboxNpcPreset;
struct FZLSocialSandboxPreset;

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
	bool bPending = false;
	FName TriggerReason;
	int32 CoalescedTriggers = 0;
	int32 AutomaticReplans = 0;
	FString ConflictLevel;
	bool bLocalFallback = false;
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

	UFUNCTION(Exec)
	void RunMultiNpcSandboxDemo();

	const TArray<TObjectPtr<AZLSocialSandboxNpc>>& GetSandboxNpcs() const { return SandboxNpcs; }
	AZLSocialSandboxNpc* FindSandboxNpc(FName StableId) const;
	FText SubmitSpeech(FName SpeechMode, FName TargetId, const FString& Text);
	FText SubmitAction(FName TargetId, const FString& Text);
	FText BeginPlayerAttack(AZLSocialSandboxPawn* Player, FName TargetId, bool bCharged);
	void ResolvePlayerAttackFromAnimNotify(AZLSocialSandboxPawn* Player);
	const FZLSocialObservationSettings& GetObservationSettings() const { return ObservationSettings; }
	const FZLSocialSandboxDecisionDebug& GetDecisionDebug() const { return DecisionDebug; }
	FText BuildInspectorText(FName NpcId) const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

private:
	void SpawnEnvironment();
	void SpawnNpc(FName StableId, const FVector& Location, const FRotator& Rotation);
	void SpawnNpc(const FZLSocialSandboxNpcPreset& Preset);
	bool TryApplyNamedPreset();
	void NotifyAcceptedAttackPresentation(AZLSocialSandboxNpc* Target, AActor* Player, const FZLSocialSandboxDamageResult& DamageResult) const;
	void DispatchActionObservation(EZLSocialActionType Action, EZLSocialActionPhase Phase, FName TargetId);
	FZLSocialObservation DispatchNpcActionObservation(AZLSocialSandboxNpc* Actor, EZLSocialActionType Action, EZLSocialActionPhase Phase, FName TargetId);
	void QueueGuardDecision(AZLSocialSandboxNpc* Guard, const FZLSocialObservation& Trigger, const FString& SpeechContent, EZLSocialSandboxDecisionTriggerReason Reason, bool bAdvanceStateVersion = false);
	void QueueNpcDecision(AZLSocialSandboxNpc* Npc, const FZLSocialObservation& Trigger, const FString& SpeechContent, EZLSocialSandboxDecisionTriggerReason Reason, bool bAdvanceStateVersion = false);
	void TryDispatchNpcDecisions();
	void RequestNpcDecision(AZLSocialSandboxNpc* Npc, const FZLSocialSandboxScheduledDecision& Scheduled);
	void HandleNpcDecision(AZLSocialSandboxNpc* Npc, const FZLDecisionResponse& Response, double SentAtSeconds);
	void HandleNpcDecisionFailure(AZLSocialSandboxNpc* Npc, const FZLServiceError& Error, double SentAtSeconds);
	void ExecuteNpcTool(AZLSocialSandboxNpc* Npc, const FZLDecisionResponse& Response);
	void TryDispatchGuardDecision();
	void SchedulePendingGuardDecision(double DelaySeconds);
	void RequestGuardDecision(AZLSocialSandboxNpc* Guard, const FZLSocialObservation& Trigger, const FString& SpeechContent, EZLSocialSandboxDecisionTriggerReason Reason);
	void HandleGuardDecision(AZLSocialSandboxNpc* Guard, const FZLDecisionResponse& Response, double SentAtSeconds);
	void HandleGuardDecisionFailure(AZLSocialSandboxNpc* Guard, const FZLServiceError& Error, double SentAtSeconds);
	void ExecuteGuardTool(AZLSocialSandboxNpc* Guard, const FZLDecisionResponse& Response);
	void RecordGuardSpeechFact(const FString& Text, double OccurredAtSeconds);
	void RecordGuardActionFact(EZLSocialActionType Action, EZLSocialActionPhase Phase, double OccurredAtSeconds);
	void UpdateGuardDistanceBand();
	void UpdateNpcDistanceBands();
	void ApplyGuardConflict(AZLSocialSandboxNpc* Guard, EZLSocialSandboxConflictEvent Event, bool bLocalFallback = false);
	void FinishDecisionSmokeTest();
	void FinishMultiNpcSmokeTest();
	void FinishPresetSmokeTest();
	void RefreshInspector() const;
	void AppendInteractionRecord(const FText& Text, const FLinearColor& Color) const;

	UPROPERTY()
	TArray<TObjectPtr<AZLSocialSandboxNpc>> SandboxNpcs;

	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Character Classes")
	TSubclassOf<AZLSocialSandboxPawn> SandboxPlayerClass;

	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Character Classes")
	TSubclassOf<AZLSocialSandboxNpc> SandboxNpcClass;

	UPROPERTY(EditDefaultsOnly, Category="Sandbox|Perception")
	FZLSocialObservationSettings ObservationSettings;
	FZLSocialToolRegistry ToolRegistry;
	FZLSocialSandboxDecisionScheduler GuardDecisionScheduler;
	FZLSocialSandboxMultiNpcDecision MultiNpcDecision;
	FZLSocialSandboxConflictState GuardConflictState;
	FZLSocialSandboxDecisionDebug DecisionDebug;
	TArray<FZLSocialSandboxPublicHistoryFact> GuardPublicHistory;
	TArray<double> GuardExecutionTimes;
	TMap<FName, FZLSocialSandboxDecisionDebug> NpcDecisionDebug;
	TMap<FName, TArray<FZLSocialSandboxPublicHistoryFact>> NpcPublicHistory;
	TMap<FName, TArray<double>> NpcExecutionTimes;
	TMap<FName, FZLSocialSandboxConflictState> NpcConflictStates;
	TMap<FName, int32> NpcDistanceBands;
	TMap<FName, float> NpcLastDistances;
	int32 GuardRequestGeneration = 0;
	FTimerHandle DemoTimer;
	FTimerHandle DecisionSmokeTimer;
	FTimerHandle MultiNpcSmokeTimer;
	FTimerHandle PresetSmokeTimer;
	FTimerHandle GuardDecisionCooldownTimer;
	FTimerHandle NpcDecisionCooldownTimer;
	FString ExpectedSmokeProvider;
	bool bExpectStaleSmoke = false;
	bool bSmokeSawAcceptedTool = false;
	FVector SmokeInitialGuardLocation = FVector::ZeroVector;
	int64 SmokeInitialGuardVersion = 0;
	int32 GuardDistanceBand = INDEX_NONE;
	float LastGuardDistance = TNumericLimits<float>::Max();
	double LastPlayerAttackSeconds = -DBL_MAX;
	FZLSocialSandboxPreset ActivePreset;
	bool bHasActivePreset = false;
};
