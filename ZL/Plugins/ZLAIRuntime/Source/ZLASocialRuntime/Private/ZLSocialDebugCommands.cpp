#include "ZLSocialDebug.h"

#include "ZLSocialTags.h"

#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogZLSocialDebug, Log, All);

static FAutoConsoleCommand GZLSocialBenchmarkCommand(
	TEXT("ZL.Social.Benchmark"),
	TEXT("Run deterministic Level 1 social simulation benchmark. Optional argument: agent count."),
	FConsoleCommandWithArgsDelegate::CreateStatic([](const TArray<FString>& Args)
	{
		const int32 Count = Args.IsEmpty() ? 120 : FCString::Atoi(*Args[0]);
		const FZLSocialBenchmarkResult Result = ZLSocialDebug::RunDeterministicBenchmark(Count);
		UE_LOG(LogZLSocialDebug, Display, TEXT("benchmark agents=%d candidates=%d perceived=%d rules=%d processing_ms=%.3f"),
			Result.RegisteredAgents, Result.Processing.Spatial.CandidatesExamined, Result.Processing.PerceivedAgents,
			Result.Processing.RuleEvaluations, Result.Processing.ProcessingMilliseconds);
	}));

static FAutoConsoleCommand GZLSocialInspectCommand(
	TEXT("ZL.Social.InspectDemo"),
	TEXT("Run a deterministic one-agent event and print the sanitized social snapshot."),
	FConsoleCommandWithArgsDelegate::CreateStatic([](const TArray<FString>& Args)
	{
		FZLSocialSimulation Simulation;
		FZLSocialAgentProfile Agent;
		Agent.AgentId = Args.IsEmpty() ? FName(TEXT("demo_agent")) : FName(*Args[0]);
		Agent.Personality.Curiosity = 0.9f;
		Simulation.RegisterAgent(Agent);
		FZLSocialEvent Event;
		Simulation.CreateEvent(ZLSocialTags::Event_Punch, TEXT("demo_source"), Agent.AgentId, FVector::ZeroVector, 1.0, Event);
		TArray<FZLSocialIntentCommand> Commands;
		FZLSocialProcessingStats Stats;
		const auto Visible = [](const FVector&, const FVector&) { return true; };
		Simulation.ProcessEvent(Event, 1.0, Commands, Stats, Visible);
		FZLSocialAgentDebugSnapshot Snapshot;
		if (Simulation.BuildDebugSnapshot(Agent.AgentId, Snapshot))
		{
			UE_LOG(LogZLSocialDebug, Display, TEXT("%s"), *ZLSocialDebug::FormatAgent(Snapshot));
		}
	}));
