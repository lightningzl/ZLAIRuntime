#include "ZLSocialDecision.h"

#include "ZLSocialTags.h"

FZLSocialRuleDecisionEngine::FZLSocialRuleDecisionEngine(const float InCooldownSeconds, const float InHysteresisMargin)
	: CooldownSeconds(FMath::Max(InCooldownSeconds, 0.0f))
	, HysteresisMargin(FMath::Max(InHysteresisMargin, 0.0f))
{
}

FZLSocialDecisionResult FZLSocialRuleDecisionEngine::Evaluate(const FZLSocialEvent& Event, const FZLSocialAgentProfile& Agent, const FZLSocialInstantState& State, const FZLSocialPerceptionResult& Perception, const double NowSeconds, FZLSocialDecisionHistory& InOutHistory) const
{
	FZLSocialDecisionResult Result;
	if (!Perception.bPerceived || !Agent.IsValid())
	{
		Result.Intent = ZLSocialTags::Intent_Ignore;
		return Result;
	}

	const bool bExtreme = Event.Type == ZLSocialTags::Event_Gunshot || Event.Type == ZLSocialTags::Event_Kill;
	const bool bHelp = Event.Type == ZLSocialTags::Event_Help;
	const FZLSocialPersonalityTraits& P = Agent.Personality;
	AddCandidate(Result.Candidates, ZLSocialTags::Intent_Ignore, 0.05f);
	AddCandidate(Result.Candidates, ZLSocialTags::Intent_Observe, bExtreme ? -100.0f : 0.2f + P.Curiosity - State.Fear * 0.3f);
	AddCandidate(Result.Candidates, ZLSocialTags::Intent_Flee, State.Fear * 1.1f + P.FearSensitivity * 0.8f - P.Brave * 0.6f + (bExtreme ? 1.0f : 0.0f));
	AddCandidate(Result.Candidates, ZLSocialTags::Intent_Report, P.Justice * 0.9f + P.Social * 0.4f + State.Alert * 0.3f, Agent.bCanReport);
	AddCandidate(Result.Candidates, ZLSocialTags::Intent_Assist, P.Social * 0.8f + P.Justice * 0.4f + (bHelp ? 0.8f : -0.3f) - State.Fear * 0.4f, Agent.bCanAssist);
	AddCandidate(Result.Candidates, ZLSocialTags::Intent_Confront, P.Brave * 0.6f + P.Aggression * 0.8f + State.Anger * 0.6f - State.Fear * 0.5f, Agent.bCanConfront && !bHelp);

	Result.Candidates.Sort([](const FZLSocialIntentScore& A, const FZLSocialIntentScore& B)
	{
		if (!FMath::IsNearlyEqual(A.Score, B.Score)) { return A.Score > B.Score; }
		return A.Intent.GetTagName().LexicalLess(B.Intent.GetTagName());
	});

	const FZLSocialIntentScore& Best = Result.Candidates[0];
	const int32 Priority = EventPriority(Event.Type);
	const bool bInCooldown = InOutHistory.CurrentIntent.IsValid() && NowSeconds - InOutHistory.LastDecisionSeconds < CooldownSeconds;
	if (bInCooldown && Priority <= InOutHistory.LastEventPriority)
	{
		Result.Intent = InOutHistory.CurrentIntent;
		Result.bHeldByCooldown = true;
		return Result;
	}

	if (InOutHistory.CurrentIntent.IsValid() && Best.Intent != InOutHistory.CurrentIntent && Best.Score < InOutHistory.CurrentScore + HysteresisMargin && Priority <= InOutHistory.LastEventPriority)
	{
		Result.Intent = InOutHistory.CurrentIntent;
		Result.bHeldByHysteresis = true;
		return Result;
	}

	Result.Intent = Best.Intent;
	InOutHistory.CurrentIntent = Best.Intent;
	InOutHistory.CurrentScore = Best.Score;
	InOutHistory.LastDecisionSeconds = NowSeconds;
	InOutHistory.LastEventPriority = Priority;
	return Result;
}

int32 FZLSocialRuleDecisionEngine::EventPriority(const FGameplayTag EventType)
{
	if (EventType == ZLSocialTags::Event_Gunshot) { return 100; }
	if (EventType == ZLSocialTags::Event_Kill) { return 90; }
	if (EventType == ZLSocialTags::Event_Punch) { return 70; }
	if (EventType == ZLSocialTags::Event_Steal) { return 50; }
	if (EventType == ZLSocialTags::Event_Help) { return 40; }
	if (EventType == ZLSocialTags::Event_Shout) { return 30; }
	return 10;
}

void FZLSocialRuleDecisionEngine::AddCandidate(TArray<FZLSocialIntentScore>& Candidates, const FGameplayTag Intent, const float Score, const bool bAllowed)
{
	if (bAllowed) { Candidates.Add({Intent, Score}); }
}
