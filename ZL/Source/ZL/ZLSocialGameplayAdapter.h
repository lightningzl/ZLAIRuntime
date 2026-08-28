#pragma once

#include "ZLSocialSimulation.h"

class FZLSocialGameplayAdapter
{
public:
	using FIntentHandler = TFunction<void(const FZLSocialIntentCommand&)>;

	bool RegisterLevel1Agent(const FZLSocialAgentProfile& Profile) { return Simulation.RegisterAgent(Profile); }
	bool RegisterImportantAgent(const FZLSocialAgentProfile& Profile) { return Profile.IsImportant() && Simulation.RegisterAgent(Profile); }
	void SetIntentHandler(FIntentHandler InHandler) { IntentHandler = MoveTemp(InHandler); }
	bool ProducePunch(FName SourceId, FName TargetId, const FVector& Position, double NowSeconds, FZLSocialProcessingStats* OutStats = nullptr, FZLSocialEvent* OutEvent = nullptr);
	bool ProduceGunshot(FName SourceId, const FVector& Position, double NowSeconds, FZLSocialProcessingStats* OutStats = nullptr);
	bool ProduceHelp(FName SourceId, FName TargetId, const FVector& Position, double NowSeconds, FZLSocialProcessingStats* OutStats = nullptr);
	bool ConfirmReport(const FZLSocialReportConfirmation& Confirmation, FZLSocialPropagationResult& OutPropagation, FZLSocialProcessingStats* OutStats = nullptr);
	bool ConfirmAuthorityAssessment(const FZLSocialEvent& SocialEvent, FName AuthorityId, double NowSeconds);

	const FZLSocialSimulation& GetSimulation() const { return Simulation; }

private:
	bool Produce(FGameplayTag Type, FName SourceId, FName TargetId, const FVector& Position, double NowSeconds, FZLSocialProcessingStats* OutStats, FZLSocialEvent* OutEvent = nullptr);
	void Deliver(const TArray<FZLSocialIntentCommand>& Commands) const;

	FZLSocialSimulation Simulation;
	FIntentHandler IntentHandler;
};
