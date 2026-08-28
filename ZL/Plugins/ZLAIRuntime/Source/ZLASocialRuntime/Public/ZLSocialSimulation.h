#pragma once

#include "CoreMinimal.h"
#include "ZLSocialDecision.h"
#include "ZLSocialEventRouter.h"
#include "ZLSocialPropagation.h"
#include "ZLSocialRelationship.h"

struct ZLASOCIALRUNTIME_API FZLSocialIntentCommand
{
	FGuid EventId;
	FGuid RootEventId;
	FGuid ParentEventId;
	FName AgentId;
	FName SubjectId;
	FGameplayTag Intent;
	EZLSocialPerceptionChannel SourceChannel = EZLSocialPerceptionChannel::None;
	float SourceConfidence = 0.0f;
	int32 ChainDepth = 0;
	int32 ChainBudget = 0;
	TArray<FZLSocialIntentScore> CandidateScores;
	TArray<FName> ReasonCodes;
};

struct ZLASOCIALRUNTIME_API FZLSocialAggregateMetrics
{
	int32 PropagationCreated = 0;
	int32 PropagationRejected = 0;
	int32 RootDuplicates = 0;
	int32 RelationshipEdges = 0;
	int32 FactionStandings = 0;
	int32 LongMemoryItems = 0;
};

struct ZLASOCIALRUNTIME_API FZLSocialProcessingStats
{
	FZLSocialSpatialQueryStats Spatial;
	int32 PerceivedAgents = 0;
	int32 RuleEvaluations = 0;
	double RuleEvaluationMilliseconds = 0.0;
	double ProcessingMilliseconds = 0.0;
	FZLSocialAggregateMetrics Aggregate;
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
	bool ConfirmFactionStanding(const FZLSocialEvent& Event, FName AuthorityId, const FZLSocialPerceptionResult& Perception, double NowSeconds);
	bool ProcessEvent(const FZLSocialEvent& Event, double NowSeconds, TArray<FZLSocialIntentCommand>& OutCommands, FZLSocialProcessingStats& OutStats, const TFunctionRef<bool(const FVector&, const FVector&)> HasLineOfSight);
	void DecayAgentStates(float DeltaSeconds);
	void Reset();

	const FZLSocialAgentState* FindAgentState(FName AgentId) const;
	const FZLSocialRelationshipStore& GetRelationshipStore() const { return RelationshipStore; }
	FZLSocialAggregateMetrics GetAggregateMetrics() const;
	bool BuildDebugSnapshot(FName AgentId, FZLSocialAgentDebugSnapshot& OutSnapshot) const;
	int32 GetRegisteredAgentCount() const { return Router.GetRegisteredAgentCount(); }

private:
	FZLSocialEventRouter Router;
	FZLSocialPerceptionFilter PerceptionFilter;
	FZLSocialRuleDecisionEngine DecisionEngine;
	FZLSocialPropagation Propagation;
	FZLSocialRelationshipStore RelationshipStore;
	TMap<FName, FZLSocialAgentProfile> Profiles;
	TMap<FName, FZLSocialAgentState> States;
	TMap<FName, FZLSocialDecisionHistory> DecisionHistories;
	TMap<FName, FZLSocialIntentCommand> LastCommands;
	int32 PropagationCreated = 0;
	int32 PropagationRejected = 0;
	int32 RootDuplicates = 0;
	int32 LongMemoryItems = 0;
};
