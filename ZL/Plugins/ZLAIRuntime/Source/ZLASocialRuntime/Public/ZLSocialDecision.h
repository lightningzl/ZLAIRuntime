#pragma once

#include "CoreMinimal.h"
#include "ZLSocialRelationship.h"
#include "ZLSocialState.h"

struct ZLASOCIALRUNTIME_API FZLSocialScoreContribution
{
	FName ReasonCode;
	float Value = 0.0f;
};

struct ZLASOCIALRUNTIME_API FZLSocialIntentScore
{
	FGameplayTag Intent;
	float Score = 0.0f;
	TArray<FZLSocialScoreContribution> Contributions;
};

struct ZLASOCIALRUNTIME_API FZLSocialDecisionResult
{
	FGameplayTag Intent;
	TArray<FZLSocialIntentScore> Candidates;
	bool bHeldByCooldown = false;
	bool bHeldByHysteresis = false;
	TArray<FName> ReasonCodes;
};

struct ZLASOCIALRUNTIME_API FZLSocialDecisionContext
{
	FZLSocialRelationshipState Relationship;
	bool bHasRelationship = false;
	float FactionStanding = 0.0f;
	bool bHasFactionStanding = false;
	int32 RelevantMemoryCount = 0;
	float StrongestMemoryImportance = 0.0f;
	float SourceConfidence = 1.0f;
	FName OccupationId;
	bool bAlreadyReportedRoot = false;
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

	FZLSocialDecisionResult Evaluate(const FZLSocialEvent& Event, const FZLSocialAgentProfile& Agent, const FZLSocialInstantState& State, const FZLSocialPerceptionResult& Perception, double NowSeconds, FZLSocialDecisionHistory& InOutHistory, const FZLSocialDecisionContext& Context = FZLSocialDecisionContext()) const;

private:
	static int32 EventPriority(FGameplayTag EventType);
	static void AddCandidate(TArray<FZLSocialIntentScore>& Candidates, FGameplayTag Intent, float Score, bool bAllowed = true);
	static void AddContribution(FZLSocialIntentScore& Candidate, FName ReasonCode, float Value);
	static void CollectReasonCodes(FZLSocialDecisionResult& Result);

	float CooldownSeconds;
	float HysteresisMargin;
};
