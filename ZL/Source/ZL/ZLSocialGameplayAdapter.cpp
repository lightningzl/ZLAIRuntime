#include "ZLSocialGameplayAdapter.h"

#include "ZLSocialTags.h"

bool FZLSocialGameplayAdapter::ProducePunch(const FName SourceId, const FName TargetId, const FVector& Position, const double NowSeconds, FZLSocialProcessingStats* OutStats, FZLSocialEvent* OutEvent)
{
	return Produce(ZLSocialTags::Event_Punch, SourceId, TargetId, Position, NowSeconds, OutStats, OutEvent);
}

bool FZLSocialGameplayAdapter::ProduceGunshot(const FName SourceId, const FVector& Position, const double NowSeconds, FZLSocialProcessingStats* OutStats)
{
	return Produce(ZLSocialTags::Event_Gunshot, SourceId, NAME_None, Position, NowSeconds, OutStats);
}

bool FZLSocialGameplayAdapter::ProduceHelp(const FName SourceId, const FName TargetId, const FVector& Position, const double NowSeconds, FZLSocialProcessingStats* OutStats)
{
	return Produce(ZLSocialTags::Event_Help, SourceId, TargetId, Position, NowSeconds, OutStats);
}

bool FZLSocialGameplayAdapter::Produce(const FGameplayTag Type, const FName SourceId, const FName TargetId, const FVector& Position, const double NowSeconds, FZLSocialProcessingStats* OutStats, FZLSocialEvent* OutEvent)
{
	FZLSocialEvent Event;
	if (!Simulation.CreateEvent(Type, SourceId, TargetId, Position, NowSeconds, Event)) { return false; }
	if (OutEvent != nullptr) { *OutEvent = Event; }
	TArray<FZLSocialIntentCommand> Commands;
	FZLSocialProcessingStats Stats;
	const auto Visible = [](const FVector&, const FVector&) { return true; };
	if (!Simulation.ProcessEvent(Event, NowSeconds, Commands, Stats, Visible)) { return false; }
	if (OutStats != nullptr) { *OutStats = Stats; }
	Deliver(Commands);
	return true;
}

bool FZLSocialGameplayAdapter::ConfirmReport(const FZLSocialReportConfirmation& Confirmation, FZLSocialPropagationResult& OutPropagation, FZLSocialProcessingStats* OutStats)
{
	if (!Simulation.ConfirmReport(Confirmation, OutPropagation)) { return false; }
	FZLSocialProcessingStats Aggregate;
	const auto Visible = [](const FVector&, const FVector&) { return true; };
	for (const FZLSocialEvent& DerivedEvent : OutPropagation.DerivedEvents)
	{
		TArray<FZLSocialIntentCommand> Commands;
		FZLSocialProcessingStats Stats;
		if (!Simulation.ProcessEvent(DerivedEvent, Confirmation.ConfirmedAtSeconds, Commands, Stats, Visible)) { return false; }
		Aggregate.Spatial.CellsVisited += Stats.Spatial.CellsVisited;
		Aggregate.Spatial.CandidatesExamined += Stats.Spatial.CandidatesExamined;
		Aggregate.Spatial.ResultsReturned += Stats.Spatial.ResultsReturned;
		Aggregate.Spatial.RegisteredAgents = FMath::Max(Aggregate.Spatial.RegisteredAgents, Stats.Spatial.RegisteredAgents);
		Aggregate.PerceivedAgents += Stats.PerceivedAgents;
		Aggregate.RuleEvaluations += Stats.RuleEvaluations;
		Aggregate.ProcessingMilliseconds += Stats.ProcessingMilliseconds;
		Deliver(Commands);
	}
	if (OutStats != nullptr) { *OutStats = Aggregate; }
	return true;
}

bool FZLSocialGameplayAdapter::ConfirmAuthorityAssessment(const FZLSocialEvent& SocialEvent, const FName AuthorityId, const double NowSeconds)
{
	if (!SocialEvent.HasChannel(EZLSocialPerceptionChannel::Social) || SocialEvent.SocialReceiverId != AuthorityId) { return false; }
	FZLSocialPerceptionResult Perception;
	Perception.bPerceived = true;
	Perception.Channel = EZLSocialPerceptionChannel::Social;
	Perception.EffectiveIntensity = SocialEvent.Confidence;
	return Simulation.ConfirmFactionStanding(SocialEvent, AuthorityId, Perception, NowSeconds);
}

void FZLSocialGameplayAdapter::Deliver(const TArray<FZLSocialIntentCommand>& Commands) const
{
	if (!IntentHandler) { return; }
	for (const FZLSocialIntentCommand& Command : Commands) { IntentHandler(Command); }
}
