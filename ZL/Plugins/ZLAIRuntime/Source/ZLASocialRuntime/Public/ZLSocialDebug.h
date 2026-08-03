#pragma once

#include "CoreMinimal.h"
#include "ZLSocialSimulation.h"

struct ZLASOCIALRUNTIME_API FZLSocialAgentDebugSnapshot
{
	FName AgentId;
	FZLSocialPersonalityTraits Personality;
	FZLSocialInstantState InstantState;
	TArray<FZLSocialMemoryEntry> ShortMemory;
	FGuid LastEventId;
	FGameplayTag FinalIntent;
	TArray<FZLSocialIntentScore> CandidateScores;
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
}
