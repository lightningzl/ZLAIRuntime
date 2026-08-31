#include "SocialSandbox/ZLSocialSandboxDecisionContext.h"

namespace
{
	FString ActionName(const EZLSocialActionType Action)
	{
		switch (Action)
		{
		case EZLSocialActionType::Face: return TEXT("Face");
		case EZLSocialActionType::Approach: return TEXT("Approach");
		case EZLSocialActionType::MoveAway: return TEXT("MoveAway");
		default: return TEXT("Stop");
		}
	}

	FString ObservationSummary(const FZLSocialObservation& Observation)
	{
		if (Observation.Source == EZLSocialObservationSource::Speech)
		{
			return TEXT("The player spoke within this NPC's hearing.");
		}
		const TCHAR* Phase = Observation.ActionPhase == EZLSocialActionPhase::Started
			? TEXT("started")
			: TEXT("completed");
		return FString::Printf(TEXT("The player %s action %s."), Phase, *ActionName(Observation.Action));
	}

	bool WasPerceived(const FZLSocialObservation& Observation)
	{
		return Observation.Source == EZLSocialObservationSource::Speech
			? Observation.bHeard
			: Observation.bSaw;
	}

	FZLDecisionAllowedTool AllowedTool(const TCHAR* Name, const bool bNeedsTarget)
	{
		FZLDecisionAllowedTool Tool;
		Tool.Name = Name;
		if (bNeedsTarget) { Tool.TargetIds.Add(TEXT("player")); }
		return Tool;
	}
}

bool FZLSocialSandboxDecisionContextBuilder::Build(
	const FZLSocialSandboxDecisionContextInput& Input,
	FZLDecisionRequest& OutRequest,
	FString& OutError)
{
	OutError.Reset();
	if (Input.NpcId.IsNone() || Input.DisplayName.IsEmptyOrWhitespace()
		|| Input.TriggerObservation.ObserverId != Input.NpcId
		|| !WasPerceived(Input.TriggerObservation))
	{
		OutError = TEXT("Decision input must contain one perceived Observation for the selected NPC");
		return false;
	}
	if (Input.TriggerObservation.Source == EZLSocialObservationSource::Speech
		&& Input.TriggerSpeechContent.TrimStartAndEnd().IsEmpty())
	{
		OutError = TEXT("Perceived speech requires bounded content");
		return false;
	}

	FZLDecisionRequest Request;
	Request.RequestId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
	Request.NpcId = Input.NpcId.ToString();
	Request.StateVersion = Input.StateVersion;
	Request.TtlMs = 30000;
	Request.Trigger.EventId = Input.TriggerObservation.EventId.ToString(EGuidFormats::DigitsWithHyphens);
	Request.Trigger.Kind = Input.TriggerObservation.Source == EZLSocialObservationSource::Speech
		? TEXT("speech")
		: TEXT("action_result");
	Request.Trigger.SourceId = Input.TriggerSourceId.ToString();
	Request.Trigger.TargetId = Input.TriggerObservation.ExplicitTargetId.ToString();
	Request.Trigger.Summary = ObservationSummary(Input.TriggerObservation);
	Request.Trigger.OccurredAtMs = FMath::Max<int64>(0, FMath::RoundToInt64(Input.TriggerObservation.ObservedAtSeconds * 1000.0));
	if (Input.TriggerObservation.Source == EZLSocialObservationSource::Speech)
	{
		Request.Trigger.Content = Input.TriggerSpeechContent.Left(512);
	}
	if (Input.TriggerObservation.bSaw) { Request.Trigger.Channels.Add(TEXT("visual")); }
	if (Input.TriggerObservation.bHeard) { Request.Trigger.Channels.Add(TEXT("auditory")); }
	if (Input.TriggerObservation.TargetJudgment == EZLSocialTargetJudgment::ExplicitSelf)
	{
		Request.Trigger.Channels.AddUnique(TEXT("direct"));
	}

	Request.Context.Npc.DisplayName = Input.DisplayName.ToString();
	Request.Context.Npc.Role = TEXT("social sandbox guard");
	Request.Context.Npc.Personality = {TEXT("cautious"), TEXT("dutiful")};
	Request.Context.Npc.SpeakingStyle = TEXT("brief and direct");
	Request.Context.Npc.Goals = {TEXT("maintain safe distance"), TEXT("keep order")};
	Request.Context.InstantState.Alert = Input.TriggerObservation.bHeardClearly || Input.TriggerObservation.bSaw ? 0.7f : 0.3f;

	const int32 StartIndex = FMath::Max(0, Input.PersonalHistory.Num() - 8);
	for (int32 Index = StartIndex; Index < Input.PersonalHistory.Num(); ++Index)
	{
		const FZLSocialObservation& Observation = Input.PersonalHistory[Index];
		if (Observation.ObserverId != Input.NpcId || !WasPerceived(Observation)
			|| Observation.EventId == Input.TriggerObservation.EventId)
		{
			continue;
		}
		FZLDecisionHistoryItem Item;
		Item.Kind = Observation.Source == EZLSocialObservationSource::Speech ? TEXT("speech") : TEXT("action_result");
		Item.SourceId = TEXT("player");
		Item.TargetId = Observation.ExplicitTargetId.ToString();
		Item.Summary = ObservationSummary(Observation);
		Item.OccurredAtMs = FMath::Max<int64>(0, FMath::RoundToInt64(Observation.ObservedAtSeconds * 1000.0));
		Request.Context.RecentHistory.Add(MoveTemp(Item));
	}

	Request.AllowedTools = {
		AllowedTool(TEXT("face_target"), true),
		AllowedTool(TEXT("move_toward"), true),
		AllowedTool(TEXT("move_away"), true),
		AllowedTool(TEXT("stop"), false)
	};
	OutRequest = MoveTemp(Request);
	return true;
}
