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
	FString Reasons;
	for (const FName Reason : Snapshot.ReasonCodes)
	{
		if (!Reasons.IsEmpty()) { Reasons += TEXT(","); }
		Reasons += Reason.ToString();
	}
	return FString::Printf(
		TEXT("agent=%s level=%d faction=%s occupation=%s traits=[brave=%.2f fear=%.2f curiosity=%.2f justice=%.2f aggression=%.2f social=%.2f] state=[fear=%.2f anger=%.2f curiosity=%.2f alert=%.2f] event=[id=%s root=%s parent=%s depth=%d budget=%d channel=%d confidence=%.2f] relationship=[present=%d trust=%.2f affinity=%.2f fear=%.2f familiarity=%.2f reputation=%.2f] faction_standing=[present=%d value=%.2f] memory=[short=%d long=%d] candidates=[%s] reasons=[%s] intent=%s"),
		*Snapshot.AgentId.ToString(), static_cast<int32>(Snapshot.AgentLevel), *Snapshot.FactionId.ToString(), *Snapshot.OccupationId.ToString(), Snapshot.Personality.Brave, Snapshot.Personality.FearSensitivity,
		Snapshot.Personality.Curiosity, Snapshot.Personality.Justice, Snapshot.Personality.Aggression, Snapshot.Personality.Social,
		Snapshot.InstantState.Fear, Snapshot.InstantState.Anger, Snapshot.InstantState.Curiosity, Snapshot.InstantState.Alert,
		*Snapshot.LastEventId.ToString(), *Snapshot.RootEventId.ToString(), *Snapshot.ParentEventId.ToString(), Snapshot.ChainDepth, Snapshot.ChainBudget,
		static_cast<int32>(Snapshot.SourceChannel), Snapshot.SourceConfidence, Snapshot.bHasRelationship ? 1 : 0, Snapshot.Relationship.Trust,
		Snapshot.Relationship.Affinity, Snapshot.Relationship.Fear, Snapshot.Relationship.Familiarity, Snapshot.Relationship.Reputation,
		Snapshot.bHasFactionStanding ? 1 : 0, Snapshot.FactionStanding.Standing, Snapshot.ShortMemory.Num(), Snapshot.LongMemory.Num(),
		*Candidates, *Reasons, *Snapshot.FinalIntent.ToString());
}

FZLSocialMilestone6BenchmarkResult ZLSocialDebug::RunMilestone6Benchmark(const int32 Level1Count, const int32 ImportantCount)
{
	FZLSocialMilestone6BenchmarkResult Result;
	FZLSocialSimulation Simulation(1000.0f);
	Result.Level1Agents = FMath::Clamp(Level1Count, 1, 10000);
	Result.ImportantAgents = FMath::Clamp(ImportantCount, 1, ZLSocialEventLimits::MaxFanOut);
	for (int32 Index = 0; Index < Result.Level1Agents; ++Index)
	{
		FZLSocialAgentProfile Agent;
		Agent.AgentId = FName(*FString::Printf(TEXT("m6_level1_%04d"), Index));
		Agent.Position = FVector((Index % 20) * 300.0f, (Index / 20) * 300.0f, 0.0f);
		Agent.Personality.Justice = 0.8f;
		Agent.Personality.Social = 0.8f;
		Simulation.RegisterAgent(Agent);
	}
	TArray<FName> ImportantIds;
	for (int32 Index = 0; Index < Result.ImportantAgents; ++Index)
	{
		FZLSocialAgentProfile Agent;
		Agent.AgentId = FName(*FString::Printf(TEXT("m6_important_%02d"), Index));
		Agent.AgentLevel = EZLSocialAgentLevel::Important;
		Agent.Position = FVector(20000.0f + Index * 100.0f, 0.0f, 0.0f);
		Agent.FactionId = TEXT("guards");
		Agent.OccupationId = TEXT("guard");
		Agent.bCanReceiveReports = true;
		Agent.bHasFactionAuthority = Index == 0;
		Simulation.RegisterAgent(Agent);
		ImportantIds.Add(Agent.AgentId);
	}
	Result.RegisteredAgents = Simulation.GetRegisteredAgentCount();

	FZLSocialEvent Root;
	Simulation.CreateEvent(ZLSocialTags::Event_Punch, TEXT("benchmark_source"), TEXT("m6_level1_0000"), FVector::ZeroVector, 1.0, Root);
	TArray<FZLSocialIntentCommand> Commands;
	const auto Visible = [](const FVector&, const FVector&) { return true; };
	Simulation.ProcessEvent(Root, 1.0, Commands, Result.Processing, Visible);
	FZLSocialReportConfirmation Confirmation;
	Confirmation.SourceEvent = Root;
	Confirmation.ReporterId = TEXT("m6_level1_0001");
	Confirmation.ReceiverIds = ImportantIds;
	Confirmation.CausationId = FGuid::NewGuid();
	Confirmation.ConfirmedAtSeconds = 1.5;
	Confirmation.bAnchorDerivedEvents = true;
	FZLSocialPropagationResult Propagation;
	if (Simulation.ConfirmReport(Confirmation, Propagation))
	{
		for (const FZLSocialEvent& SocialEvent : Propagation.DerivedEvents)
		{
			FZLSocialProcessingStats SocialStats;
			Simulation.ProcessEvent(SocialEvent, 1.5, Commands, SocialStats, Visible);
			Result.Processing.Spatial.CellsVisited += SocialStats.Spatial.CellsVisited;
			Result.Processing.Spatial.CandidatesExamined += SocialStats.Spatial.CandidatesExamined;
			Result.Processing.Spatial.ResultsReturned += SocialStats.Spatial.ResultsReturned;
			Result.Processing.PerceivedAgents += SocialStats.PerceivedAgents;
			Result.Processing.RuleEvaluations += SocialStats.RuleEvaluations;
			Result.Processing.RuleEvaluationMilliseconds += SocialStats.RuleEvaluationMilliseconds;
			Result.Processing.ProcessingMilliseconds += SocialStats.ProcessingMilliseconds;
		}
		FZLSocialPerceptionResult AuthorityPerception;
		AuthorityPerception.bPerceived = true;
		AuthorityPerception.Channel = EZLSocialPerceptionChannel::Social;
		AuthorityPerception.EffectiveIntensity = Propagation.DerivedEvents[0].Confidence;
		Simulation.ConfirmFactionStanding(Propagation.DerivedEvents[0], ImportantIds[0], AuthorityPerception, 1.5);
	}
	Result.Aggregate = Simulation.GetAggregateMetrics();
	Result.Processing.Aggregate = Result.Aggregate;
	return Result;
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
