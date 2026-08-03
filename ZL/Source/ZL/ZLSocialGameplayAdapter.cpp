#include "ZLSocialGameplayAdapter.h"

#include "ZLSocialTags.h"

bool FZLSocialGameplayAdapter::ProducePunch(const FName SourceId, const FName TargetId, const FVector& Position, const double NowSeconds, FZLSocialProcessingStats* OutStats)
{
	return Produce(ZLSocialTags::Event_Punch, SourceId, TargetId, Position, NowSeconds, OutStats);
}

bool FZLSocialGameplayAdapter::ProduceGunshot(const FName SourceId, const FVector& Position, const double NowSeconds, FZLSocialProcessingStats* OutStats)
{
	return Produce(ZLSocialTags::Event_Gunshot, SourceId, NAME_None, Position, NowSeconds, OutStats);
}

bool FZLSocialGameplayAdapter::ProduceHelp(const FName SourceId, const FName TargetId, const FVector& Position, const double NowSeconds, FZLSocialProcessingStats* OutStats)
{
	return Produce(ZLSocialTags::Event_Help, SourceId, TargetId, Position, NowSeconds, OutStats);
}

bool FZLSocialGameplayAdapter::Produce(const FGameplayTag Type, const FName SourceId, const FName TargetId, const FVector& Position, const double NowSeconds, FZLSocialProcessingStats* OutStats)
{
	FZLSocialEvent Event;
	if (!Simulation.CreateEvent(Type, SourceId, TargetId, Position, NowSeconds, Event)) { return false; }
	TArray<FZLSocialIntentCommand> Commands;
	FZLSocialProcessingStats Stats;
	const auto Visible = [](const FVector&, const FVector&) { return true; };
	if (!Simulation.ProcessEvent(Event, NowSeconds, Commands, Stats, Visible)) { return false; }
	if (OutStats != nullptr) { *OutStats = Stats; }
	if (IntentHandler)
	{
		for (const FZLSocialIntentCommand& Command : Commands) { IntentHandler(Command); }
	}
	return true;
}
