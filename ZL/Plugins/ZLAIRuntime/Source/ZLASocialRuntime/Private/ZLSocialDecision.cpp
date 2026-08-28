#include "ZLSocialDecision.h"

#include "ZLSocialTags.h"

FZLSocialRuleDecisionEngine::FZLSocialRuleDecisionEngine(const float InCooldownSeconds, const float InHysteresisMargin)
	: CooldownSeconds(FMath::Max(InCooldownSeconds, 0.0f))
	, HysteresisMargin(FMath::Max(InHysteresisMargin, 0.0f))
{
}

FZLSocialDecisionResult FZLSocialRuleDecisionEngine::Evaluate(const FZLSocialEvent& Event, const FZLSocialAgentProfile& Agent, const FZLSocialInstantState& State, const FZLSocialPerceptionResult& Perception, const double NowSeconds, FZLSocialDecisionHistory& InOutHistory, const FZLSocialDecisionContext& Context) const
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
	AddCandidate(Result.Candidates, ZLSocialTags::Intent_Investigate, bExtreme ? -100.0f : 0.25f + P.Curiosity * 0.5f + State.Alert * 0.5f, Agent.IsImportant() || Agent.bCanReport);
	AddCandidate(Result.Candidates, ZLSocialTags::Intent_Flee, State.Fear * 1.1f + P.FearSensitivity * 0.8f - P.Brave * 0.6f + (bExtreme ? 1.0f : 0.0f));
	AddCandidate(Result.Candidates, ZLSocialTags::Intent_Report, P.Justice * 0.9f + P.Social * 0.4f + State.Alert * 0.3f, Agent.bCanReport && !Context.bAlreadyReportedRoot);
	AddCandidate(Result.Candidates, ZLSocialTags::Intent_Assist, P.Social * 0.8f + P.Justice * 0.4f + (bHelp ? 0.8f : -0.3f) - State.Fear * 0.4f, Agent.bCanAssist);
	AddCandidate(Result.Candidates, ZLSocialTags::Intent_Confront, P.Brave * 0.6f + P.Aggression * 0.8f + State.Anger * 0.6f - State.Fear * 0.5f, Agent.bCanConfront && !bHelp);

	for (FZLSocialIntentScore& Candidate : Result.Candidates)
	{
		AddContribution(Candidate, TEXT("base.utility"), Candidate.Score);
		if (Candidate.Intent == ZLSocialTags::Intent_Investigate)
		{
			AddContribution(Candidate, TEXT("source.uncertainty"), (1.0f - FMath::Clamp(Context.SourceConfidence, 0.0f, 1.0f)) * 0.8f);
			AddContribution(Candidate, TEXT("memory.relevance"), FMath::Min(Context.RelevantMemoryCount, 4) * 0.05f + Context.StrongestMemoryImportance * 0.15f);
			if (Context.OccupationId == TEXT("guard")) { AddContribution(Candidate, TEXT("occupation.guard"), 0.35f); }
		}
		if (Candidate.Intent == ZLSocialTags::Intent_Assist && Context.bHasRelationship)
		{
			const float Support = FMath::Max(0.0f, Context.Relationship.Trust + Context.Relationship.Affinity + Context.Relationship.Reputation);
			AddContribution(Candidate, TEXT("relationship.support"), Support * 0.55f);
		}
		if (Candidate.Intent == ZLSocialTags::Intent_Confront && Context.bHasRelationship)
		{
			const float Threat = FMath::Max(0.0f, -Context.Relationship.Trust - Context.Relationship.Affinity + Context.Relationship.Fear - Context.Relationship.Reputation);
			AddContribution(Candidate, TEXT("relationship.threat"), Threat * 0.45f);
		}
		if ((Candidate.Intent == ZLSocialTags::Intent_Confront || Candidate.Intent == ZLSocialTags::Intent_Report) && Context.bHasFactionStanding)
		{
			AddContribution(Candidate, TEXT("faction.standing"), FMath::Max(0.0f, -Context.FactionStanding) * (Candidate.Intent == ZLSocialTags::Intent_Confront ? 0.4f : 0.2f));
		}
		if (Candidate.Intent == ZLSocialTags::Intent_Report && Context.OccupationId == TEXT("guard"))
		{
			AddContribution(Candidate, TEXT("occupation.guard"), 0.2f);
		}
	}

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
		CollectReasonCodes(Result);
		return Result;
	}

	if (InOutHistory.CurrentIntent.IsValid() && Best.Intent != InOutHistory.CurrentIntent && Best.Score < InOutHistory.CurrentScore + HysteresisMargin && Priority <= InOutHistory.LastEventPriority)
	{
		Result.Intent = InOutHistory.CurrentIntent;
		Result.bHeldByHysteresis = true;
		CollectReasonCodes(Result);
		return Result;
	}

	Result.Intent = Best.Intent;
	InOutHistory.CurrentIntent = Best.Intent;
	InOutHistory.CurrentScore = Best.Score;
	InOutHistory.LastDecisionSeconds = NowSeconds;
	InOutHistory.LastEventPriority = Priority;
	CollectReasonCodes(Result);
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

void FZLSocialRuleDecisionEngine::AddContribution(FZLSocialIntentScore& Candidate, const FName ReasonCode, const float Value)
{
	if (FMath::IsNearlyZero(Value)) { return; }
	Candidate.Contributions.Add({ReasonCode, Value});
	if (ReasonCode != TEXT("base.utility")) { Candidate.Score += Value; }
}

void FZLSocialRuleDecisionEngine::CollectReasonCodes(FZLSocialDecisionResult& Result)
{
	for (const FZLSocialIntentScore& Candidate : Result.Candidates)
	{
		if (Candidate.Intent != Result.Intent) { continue; }
		for (const FZLSocialScoreContribution& Contribution : Candidate.Contributions)
		{
			Result.ReasonCodes.Add(Contribution.ReasonCode);
		}
		return;
	}
}
