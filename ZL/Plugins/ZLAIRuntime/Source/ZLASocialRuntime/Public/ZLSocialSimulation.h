#pragma once

#include "CoreMinimal.h"
#include "ZLSocialDecision.h"
#include "ZLSocialEventRouter.h"
#include "ZLSocialPropagation.h"

struct ZLASOCIALRUNTIME_API FZLSocialIntentCommand
{
	FGuid EventId;
	FName AgentId;
	FGameplayTag Intent;
	TArray<FZLSocialIntentScore> CandidateScores;
};

struct ZLASOCIALRUNTIME_API FZLSocialProcessingStats
{
	FZLSocialSpatialQueryStats Spatial;
	int32 PerceivedAgents = 0;
	int32 RuleEvaluations = 0;
	double ProcessingMilliseconds = 0.0;
};

struct FZLSocialAgentDebugSnapshot;

class ZLASOCIALRUNTIME_API FZLSocialSimulation
{
public:
	explicit FZLSocialSimulation(float CellSize = 1000.0f);

	bool RegisterAgent(const FZLSocialAgentProfile& Profile);
	bool UpdateAgentPosition(FName AgentId, const FVector& Position);
	bool UnregisterAgent(FName AgentId);
	bool CreateEvent(FGameplayTag Type, FName SourceId, FName TargetId, const FVector& Position, double NowSeconds, FZLSocialEvent& OutEvent) const;
	bool ConfirmReport(const FZLSocialReportConfirmation& Confirmation, FZLSocialPropagationResult& OutResult);
	bool ProcessEvent(const FZLSocialEvent& Event, double NowSeconds, TArray<FZLSocialIntentCommand>& OutCommands, FZLSocialProcessingStats& OutStats, const TFunctionRef<bool(const FVector&, const FVector&)> HasLineOfSight);
	void DecayAgentStates(float DeltaSeconds);
	void Reset();

	const FZLSocialAgentState* FindAgentState(FName AgentId) const;
	bool BuildDebugSnapshot(FName AgentId, FZLSocialAgentDebugSnapshot& OutSnapshot) const;
	int32 GetRegisteredAgentCount() const { return Router.GetRegisteredAgentCount(); }

private:
	FZLSocialEventRouter Router;
	FZLSocialPerceptionFilter PerceptionFilter;
	FZLSocialRuleDecisionEngine DecisionEngine;
	FZLSocialPropagation Propagation;
	TMap<FName, FZLSocialAgentProfile> Profiles;
	TMap<FName, FZLSocialAgentState> States;
	TMap<FName, FZLSocialDecisionHistory> DecisionHistories;
	TMap<FName, FZLSocialIntentCommand> LastCommands;
};
