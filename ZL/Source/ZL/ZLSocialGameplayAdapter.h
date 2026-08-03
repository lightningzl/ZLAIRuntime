#pragma once

#include "ZLSocialSimulation.h"

class FZLSocialGameplayAdapter
{
public:
	using FIntentHandler = TFunction<void(const FZLSocialIntentCommand&)>;

	bool RegisterLevel1Agent(const FZLSocialAgentProfile& Profile) { return Simulation.RegisterAgent(Profile); }
	void SetIntentHandler(FIntentHandler InHandler) { IntentHandler = MoveTemp(InHandler); }
	bool ProducePunch(FName SourceId, FName TargetId, const FVector& Position, double NowSeconds, FZLSocialProcessingStats* OutStats = nullptr);
	bool ProduceGunshot(FName SourceId, const FVector& Position, double NowSeconds, FZLSocialProcessingStats* OutStats = nullptr);
	bool ProduceHelp(FName SourceId, FName TargetId, const FVector& Position, double NowSeconds, FZLSocialProcessingStats* OutStats = nullptr);

	const FZLSocialSimulation& GetSimulation() const { return Simulation; }

private:
	bool Produce(FGameplayTag Type, FName SourceId, FName TargetId, const FVector& Position, double NowSeconds, FZLSocialProcessingStats* OutStats);

	FZLSocialSimulation Simulation;
	FIntentHandler IntentHandler;
};
