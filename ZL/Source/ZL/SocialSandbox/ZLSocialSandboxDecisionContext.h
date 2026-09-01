#pragma once

#include "CoreMinimal.h"
#include "ZLAIServiceTypes.h"
#include "ZLSocialObservation.h"

struct FZLSocialSandboxDecisionContextInput
{
	FName NpcId;
	FText DisplayName;
	FName TriggerSourceId = TEXT("player");
	FZLSocialObservation TriggerObservation;
	FString TriggerSpeechContent;
	TArray<FZLSocialObservation> PersonalHistory;
	int64 StateVersion = 0;
};

class FZLSocialSandboxDecisionContextBuilder
{
public:
	static bool Build(
		const FZLSocialSandboxDecisionContextInput& Input,
		FZLDecisionRequest& OutRequest,
		FString& OutError);
};
