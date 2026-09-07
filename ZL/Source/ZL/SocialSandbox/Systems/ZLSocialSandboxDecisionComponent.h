#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZLSocialToolRegistry.h"
#include "SocialSandbox/Decision/ZLSocialSandboxDecisionContext.h"
#include "SocialSandbox/Decision/ZLSocialSandboxMultiNpcDecision.h"
#include "SocialSandbox/Decision/ZLSocialSandboxDecisionState.h"
#include "SocialSandbox/Domain/ZLSocialSandboxConflictState.h"
#include "ZLSocialSandboxDecisionComponent.generated.h"

class AZLSocialSandboxGameMode;
class AZLSocialSandboxNpc;
struct FZLSocialObservation;
struct FZLSocialSandboxScheduledDecision;
struct FZLDecisionV2Response;
struct FZLDecisionV2Request;
struct FZLDecisionV2Capability;
struct FZLDecisionV2PlanStep;
struct FZLDecisionResponse;
struct FZLServiceError;

/** 决策组件：管理 NPC 决策队列、执行限流、冲突状态与调试快照。 */
UCLASS(ClassGroup=(SocialSandbox), meta=(BlueprintSpawnableComponent))
class ZL_API UZLSocialSandboxDecisionComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	/** 初始化受控工具注册表与组件运行状态。 */
	UZLSocialSandboxDecisionComponent();
	/** 清空决策运行状态，并使未完成请求失效。 */
	void ResetState();
	void QueueNpcDecision(AZLSocialSandboxNpc* Npc, const FZLSocialObservation& Trigger, const FString& SpeechContent, EZLSocialSandboxDecisionTriggerReason Reason, bool bAdvanceStateVersion = false);
	/** 按规则事件更新 NPC 冲突等级及防御姿态。 */
	void ApplyNpcConflict(AZLSocialSandboxNpc* Npc, EZLSocialSandboxConflictEvent Event, bool bLocalFallback = false);
	/** 记录供下一轮决策使用、数量受限的社交事实。 */
	void RecordNpcSocialFact(FName NpcId, FString Kind, FName SubjectId, FName TargetId, FString Summary, float Salience);

	FZLSocialToolRegistry ToolRegistry;
	FZLSocialSandboxMultiNpcDecision MultiNpcDecision;
	TMap<FName, FZLSocialSandboxDecisionDebug> NpcDecisionDebug;
	TMap<FName, TArray<FZLSocialSandboxPublicHistoryFact>> NpcPublicHistory;
	TMap<FName, TArray<FZLDecisionV2SocialFact>> NpcSocialFacts;
	TMap<FName, FString> NpcTradeStances;
	TMap<FName, TArray<double>> NpcExecutionTimes;
	TMap<FName, FZLSocialSandboxConflictState> NpcConflictStates;
	TMap<FName, int32> NpcDistanceBands;
	TMap<FName, float> NpcLastDistances;
	int32 RequestGeneration = 0;
	FTimerHandle CooldownTimer;

private:
	AZLSocialSandboxGameMode* GetSandboxGameMode() const;
	AZLSocialSandboxNpc* FindSandboxNpc(FName StableId) const;
	FZLSocialObservation DispatchNpcActionObservation(AZLSocialSandboxNpc* Actor, EZLSocialActionType Action, EZLSocialActionPhase Phase, FName TargetId);
	void RefreshInspector() const;
	void AppendInteractionRecord(const FText& Text, const FLinearColor& Color) const;
	void TryDispatchNpcDecisions();
	void RequestNpcDecision(AZLSocialSandboxNpc* Npc, const FZLSocialSandboxScheduledDecision& Scheduled);
	void HandleNpcDecisionV2(AZLSocialSandboxNpc* Npc, const FZLDecisionV2Response& Response, const FZLDecisionV2Request& Request, double SentAtSeconds);
	void HandleNpcDecision(AZLSocialSandboxNpc* Npc, const FZLDecisionResponse& Response, double SentAtSeconds);
	void HandleNpcDecisionFailure(AZLSocialSandboxNpc* Npc, const FZLServiceError& Error, double SentAtSeconds);
	void ExecuteNpcTool(AZLSocialSandboxNpc* Npc, const FZLDecisionResponse& Response);
	void ExecuteNpcPlanStep(AZLSocialSandboxNpc* Npc, const FZLDecisionV2Response& Response, const FZLDecisionV2Capability& Capability, const FZLDecisionV2PlanStep& Step);
};
