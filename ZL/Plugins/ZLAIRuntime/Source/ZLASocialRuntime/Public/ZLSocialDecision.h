#pragma once

#include "CoreMinimal.h"
#include "ZLSocialState.h"

struct ZLASOCIALRUNTIME_API FZLSocialIntentScore
{
	FGameplayTag Intent;
	float Score = 0.0f;
};

struct ZLASOCIALRUNTIME_API FZLSocialDecisionResult
{
	FGameplayTag Intent;
	TArray<FZLSocialIntentScore> Candidates;
	bool bHeldByCooldown = false;
	bool bHeldByHysteresis = false;
};

struct ZLASOCIALRUNTIME_API FZLSocialDecisionHistory
{
	FGameplayTag CurrentIntent;
	float CurrentScore = 0.0f;
	double LastDecisionSeconds = -TNumericLimits<double>::Max();
	int32 LastEventPriority = 0;
};

class ZLASOCIALRUNTIME_API FZLSocialRuleDecisionEngine
{
public:
	explicit FZLSocialRuleDecisionEngine(float InCooldownSeconds = 1.5f, float InHysteresisMargin = 0.15f);

	FZLSocialDecisionResult Evaluate(const FZLSocialEvent& Event, const FZLSocialAgentProfile& Agent, const FZLSocialInstantState& State, const FZLSocialPerceptionResult& Perception, double NowSeconds, FZLSocialDecisionHistory& InOutHistory) const;

private:
	static int32 EventPriority(FGameplayTag EventType);
	static void AddCandidate(TArray<FZLSocialIntentScore>& Candidates, FGameplayTag Intent, float Score, bool bAllowed = true);

	float CooldownSeconds;
	float HysteresisMargin;
};
