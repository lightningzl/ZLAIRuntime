#include "ZLSocialDebug.h"

#include "ZLSocialTags.h"

FString ZLSocialDebug::FormatAgent(const FZLSocialAgentDebugSnapshot& Snapshot)
{
	FString Candidates;
	for (const FZLSocialIntentScore& Candidate : Snapshot.CandidateScores)
	{
		if (!Candidates.IsEmpty()) { Candidates += TEXT(","); }
		Candidates += FString::Printf(TEXT("%s=%.3f"), *Candidate.Intent.ToString(), Candidate.Score);
	}
	return FString::Printf(
		TEXT("agent=%s traits=[brave=%.2f fear=%.2f curiosity=%.2f justice=%.2f aggression=%.2f social=%.2f] state=[fear=%.2f anger=%.2f curiosity=%.2f alert=%.2f] last_event=%s memory=%d candidates=[%s] intent=%s"),
		*Snapshot.AgentId.ToString(), Snapshot.Personality.Brave, Snapshot.Personality.FearSensitivity,
		Snapshot.Personality.Curiosity, Snapshot.Personality.Justice, Snapshot.Personality.Aggression, Snapshot.Personality.Social,
		Snapshot.InstantState.Fear, Snapshot.InstantState.Anger, Snapshot.InstantState.Curiosity, Snapshot.InstantState.Alert,
		*Snapshot.LastEventId.ToString(), Snapshot.ShortMemory.Num(), *Candidates, *Snapshot.FinalIntent.ToString());
}

FZLSocialBenchmarkResult ZLSocialDebug::RunDeterministicBenchmark(const int32 AgentCount)
{
	FZLSocialBenchmarkResult Result;
	FZLSocialSimulation Simulation(1000.0f);
	const int32 BoundedCount = FMath::Clamp(AgentCount, 1, 10000);
	for (int32 Index = 0; Index < BoundedCount; ++Index)
	{
		FZLSocialAgentProfile Agent;
		Agent.AgentId = FName(*FString::Printf(TEXT("benchmark_%04d"), Index));
		Agent.Position = FVector((Index % 20) * 300.0f, (Index / 20) * 300.0f, 0.0f);
		Agent.Personality.Brave = static_cast<float>(Index % 5) / 4.0f;
		Agent.Personality.FearSensitivity = static_cast<float>((Index + 1) % 5) / 4.0f;
		Agent.Personality.Curiosity = static_cast<float>((Index + 2) % 5) / 4.0f;
		Agent.Personality.Justice = static_cast<float>((Index + 3) % 5) / 4.0f;
		Agent.Personality.Aggression = static_cast<float>((Index + 4) % 5) / 4.0f;
		Simulation.RegisterAgent(Agent);
	}
	Result.RegisteredAgents = Simulation.GetRegisteredAgentCount();
	FZLSocialEvent Event;
	Simulation.CreateEvent(ZLSocialTags::Event_Gunshot, TEXT("benchmark_source"), NAME_None, FVector::ZeroVector, 1.0, Event);
	TArray<FZLSocialIntentCommand> Commands;
	const auto Visible = [](const FVector&, const FVector&) { return true; };
	Simulation.ProcessEvent(Event, 1.0, Commands, Result.Processing, Visible);
	return Result;
}
