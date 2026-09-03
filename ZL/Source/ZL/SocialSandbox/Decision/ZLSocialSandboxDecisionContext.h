#pragma once

#include "CoreMinimal.h"
#include "ZLAIServiceTypes.h"
#include "ZLSocialObservation.h"
#include "SocialSandbox/Domain/ZLSocialSandboxNpcProfile.h"

struct FZLSocialSandboxPublicHistoryFact
{
	FString Kind;
	FName SourceId;
	FName TargetId;
	FString Summary;
	double OccurredAtSeconds = 0.0;
};

struct FZLSocialSandboxDecisionContextInput
{
	FName NpcId;
	FText DisplayName;
	FZLSocialSandboxNpcProfile Profile;
	FName TriggerSourceId = TEXT("player");
	FZLSocialObservation TriggerObservation;
	FString TriggerSpeechContent;
	TArray<FZLSocialObservation> PersonalHistory;
	TArray<FZLSocialSandboxPublicHistoryFact> PublicHistory;
	// UE-authoritative, personally available social facts.  The caller may append
	// facts received through a confirmed report; this builder never reads another
	// NPC's private observations.
	TArray<FZLDecisionV2SocialFact> SocialSituation;
	TArray<FZLDecisionV2Capability> AvailableCapabilities;
	int64 StateVersion = 0;
};

class FZLSocialSandboxDecisionContextBuilder
{
public:
	static bool Build(
		const FZLSocialSandboxDecisionContextInput& Input,
		FZLDecisionRequest& OutRequest,
		FString& OutError);

	static bool BuildV2(
		const FZLSocialSandboxDecisionContextInput& Input,
		FZLDecisionV2Request& OutRequest,
		FString& OutError);
};
