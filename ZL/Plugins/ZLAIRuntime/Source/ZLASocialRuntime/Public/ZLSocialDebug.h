#pragma once

#include "CoreMinimal.h"
#include "ZLSocialSimulation.h"

struct ZLASOCIALRUNTIME_API FZLSocialAgentDebugSnapshot
{
	FName AgentId;
	EZLSocialAgentLevel AgentLevel = EZLSocialAgentLevel::Level1;
	FName FactionId;
	FName OccupationId;
	FZLSocialPersonalityTraits Personality;
	FZLSocialInstantState InstantState;
	TArray<FZLSocialMemoryEntry> ShortMemory;
	TArray<FZLSocialMemoryEntry> LongMemory;
	FGuid LastEventId;
	FGuid RootEventId;
	FGuid ParentEventId;
	FName SubjectId;
	int32 ChainDepth = 0;
	int32 ChainBudget = 0;
	EZLSocialPerceptionChannel SourceChannel = EZLSocialPerceptionChannel::None;
	float SourceConfidence = 0.0f;
	FZLSocialRelationshipState Relationship;
	bool bHasRelationship = false;
	FZLSocialFactionStandingState FactionStanding;
	bool bHasFactionStanding = false;
	FGameplayTag FinalIntent;
	TArray<FZLSocialIntentScore> CandidateScores;
	TArray<FName> ReasonCodes;
};

struct ZLASOCIALRUNTIME_API FZLSocialMilestone6BenchmarkResult
{
	int32 Level1Agents = 0;
	int32 ImportantAgents = 0;
	int32 RegisteredAgents = 0;
	FZLSocialProcessingStats Processing;
	FZLSocialAggregateMetrics Aggregate;
};

struct ZLASOCIALRUNTIME_API FZLSocialBenchmarkResult
{
	int32 RegisteredAgents = 0;
	FZLSocialProcessingStats Processing;
};

namespace ZLSocialDebug
{
	ZLASOCIALRUNTIME_API FString FormatAgent(const FZLSocialAgentDebugSnapshot& Snapshot);
	ZLASOCIALRUNTIME_API FZLSocialBenchmarkResult RunDeterministicBenchmark(int32 AgentCount = 120);
	ZLASOCIALRUNTIME_API FZLSocialMilestone6BenchmarkResult RunMilestone6Benchmark(int32 Level1Count = 120, int32 ImportantCount = 5);
}
