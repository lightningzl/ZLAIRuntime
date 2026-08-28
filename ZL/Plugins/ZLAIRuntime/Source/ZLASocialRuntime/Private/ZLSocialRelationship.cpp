#include "ZLSocialRelationship.h"

#include "ZLSocialTags.h"

void FZLSocialRelationshipState::Clamp()
{
	Trust = FMath::Clamp(Trust, -1.0f, 1.0f);
	Affinity = FMath::Clamp(Affinity, -1.0f, 1.0f);
	Fear = FMath::Clamp(Fear, 0.0f, 1.0f);
	Familiarity = FMath::Clamp(Familiarity, 0.0f, 1.0f);
	Reputation = FMath::Clamp(Reputation, -1.0f, 1.0f);
}

bool FZLSocialRelationshipStore::ApplyPersonalEvent(const FZLSocialEvent& Event, const FZLSocialAgentProfile& Observer, const FZLSocialPerceptionResult& Perception, const double NowSeconds, FZLSocialRelationshipDelta* OutDelta)
{
	if (!Event.IsValid(NowSeconds) || !Observer.IsValid() || !Perception.bPerceived || Event.SourceId.IsNone() || Event.SourceId == Observer.AgentId)
	{
		return false;
	}
	for (auto Iterator = RootExpiryByRoot.CreateIterator(); Iterator; ++Iterator)
	{
		if (Iterator.Value() < NowSeconds)
		{
			PersonalObserversByRoot.Remove(Iterator.Key());
			UpdatedFactionsByRoot.Remove(Iterator.Key());
			Iterator.RemoveCurrent();
		}
	}
	if (!RootExpiryByRoot.Contains(Event.RootEventId))
	{
		if (RootExpiryByRoot.Num() >= ZLSocialRuntimeLimits::MaxTrackedRoots) { return false; }
		RootExpiryByRoot.Add(Event.RootEventId, Event.ExpiresAtSeconds);
	}
	TSet<FName>& Observers = PersonalObserversByRoot.FindOrAdd(Event.RootEventId);
	if (Observers.Contains(Observer.AgentId))
	{
		return false;
	}

	FZLSocialRelationshipDelta Delta;
	if (!BuildDelta(Event, Observer, Perception, Delta))
	{
		return false;
	}
	const FZLSocialRelationshipKey Key{Observer.AgentId, Event.SourceId};
	if (!Relationships.Contains(Key) && Relationships.Num() >= ZLSocialRuntimeLimits::MaxRelationshipEdges) { return false; }
	FZLSocialRelationshipState& State = Relationships.FindOrAdd(Key);
	State.Trust += Delta.Trust;
	State.Affinity += Delta.Affinity;
	State.Fear += Delta.Fear;
	State.Familiarity += Delta.Familiarity;
	State.Reputation += Delta.Reputation;
	State.LastUpdatedSeconds = NowSeconds;
	State.LastCausationEventId = Event.RootEventId;
	State.Clamp();
	Observers.Add(Observer.AgentId);
	if (OutDelta != nullptr)
	{
		*OutDelta = Delta;
	}
	return true;
}

bool FZLSocialRelationshipStore::ConfirmFactionStanding(const FZLSocialEvent& Event, const FZLSocialAgentProfile& Authority, const FZLSocialPerceptionResult& Perception, const double NowSeconds)
{
	if (!Event.IsValid(NowSeconds) || !Authority.IsValid() || !Authority.IsImportant() || !Authority.bHasFactionAuthority
		|| Authority.FactionId.IsNone() || !Perception.bPerceived || Perception.EffectiveIntensity < 0.6f || Event.SourceId.IsNone())
	{
		return false;
	}
	for (auto Iterator = RootExpiryByRoot.CreateIterator(); Iterator; ++Iterator)
	{
		if (Iterator.Value() < NowSeconds)
		{
			PersonalObserversByRoot.Remove(Iterator.Key());
			UpdatedFactionsByRoot.Remove(Iterator.Key());
			Iterator.RemoveCurrent();
		}
	}
	if (!RootExpiryByRoot.Contains(Event.RootEventId))
	{
		if (RootExpiryByRoot.Num() >= ZLSocialRuntimeLimits::MaxTrackedRoots) { return false; }
		RootExpiryByRoot.Add(Event.RootEventId, Event.ExpiresAtSeconds);
	}
	TSet<FName>& UpdatedFactions = UpdatedFactionsByRoot.FindOrAdd(Event.RootEventId);
	if (UpdatedFactions.Contains(Authority.FactionId))
	{
		return false;
	}

	float BaseDelta = 0.0f;
	if (Event.Type == ZLSocialTags::Event_Punch) { BaseDelta = -0.35f; }
	else if (Event.Type == ZLSocialTags::Event_Help) { BaseDelta = 0.25f; }
	else { return false; }

	const FZLSocialFactionStandingKey Key{Authority.FactionId, Event.SourceId};
	if (!FactionStandings.Contains(Key) && FactionStandings.Num() >= ZLSocialRuntimeLimits::MaxFactionStandings) { return false; }
	FZLSocialFactionStandingState& State = FactionStandings.FindOrAdd(Key);
	State.Standing = FMath::Clamp(State.Standing + BaseDelta * SourceWeight(Perception.Channel, Perception.EffectiveIntensity), -1.0f, 1.0f);
	State.LastUpdatedSeconds = NowSeconds;
	State.LastCausationEventId = Event.RootEventId;
	UpdatedFactions.Add(Authority.FactionId);
	return true;
}

void FZLSocialRelationshipStore::DecayTowardsNeutral(const float DeltaSeconds, const float RatePerSecond)
{
	const float Amount = FMath::Max(DeltaSeconds, 0.0f) * FMath::Max(RatePerSecond, 0.0f);
	for (TPair<FZLSocialRelationshipKey, FZLSocialRelationshipState>& Pair : Relationships)
	{
		FZLSocialRelationshipState& State = Pair.Value;
		State.Trust = FMath::FInterpConstantTo(State.Trust, 0.0f, 1.0f, Amount);
		State.Affinity = FMath::FInterpConstantTo(State.Affinity, 0.0f, 1.0f, Amount);
		State.Fear = FMath::FInterpConstantTo(State.Fear, 0.0f, 1.0f, Amount);
		State.Reputation = FMath::FInterpConstantTo(State.Reputation, 0.0f, 1.0f, Amount);
		State.Familiarity = FMath::FInterpConstantTo(State.Familiarity, 0.0f, 1.0f, Amount * 0.25f);
		State.Clamp();
	}
}

void FZLSocialRelationshipStore::Reset()
{
	Relationships.Reset();
	FactionStandings.Reset();
	PersonalObserversByRoot.Reset();
	UpdatedFactionsByRoot.Reset();
	RootExpiryByRoot.Reset();
}

const FZLSocialRelationshipState* FZLSocialRelationshipStore::FindRelationship(const FName ObserverId, const FName SubjectId) const
{
	return Relationships.Find({ObserverId, SubjectId});
}

const FZLSocialFactionStandingState* FZLSocialRelationshipStore::FindFactionStanding(const FName FactionId, const FName SubjectId) const
{
	return FactionStandings.Find({FactionId, SubjectId});
}

bool FZLSocialRelationshipStore::BuildDelta(const FZLSocialEvent& Event, const FZLSocialAgentProfile& Observer, const FZLSocialPerceptionResult& Perception, FZLSocialRelationshipDelta& OutDelta)
{
	if (Event.Type == ZLSocialTags::Event_Punch)
	{
		OutDelta.Trust = -0.4f;
		OutDelta.Affinity = -0.25f;
		OutDelta.Fear = 0.3f;
		OutDelta.Familiarity = 0.2f;
		OutDelta.Reputation = -0.35f;
	}
	else if (Event.Type == ZLSocialTags::Event_Help)
	{
		OutDelta.Trust = 0.35f;
		OutDelta.Affinity = 0.25f;
		OutDelta.Fear = -0.1f;
		OutDelta.Familiarity = 0.15f;
		OutDelta.Reputation = 0.2f;
	}
	else
	{
		return false;
	}

	const float PersonalRelevance = Event.TargetId == Observer.AgentId ? 1.0f : 0.65f;
	const float Weight = PersonalRelevance * SourceWeight(Perception.Channel, Perception.EffectiveIntensity);
	OutDelta.Trust *= Weight;
	OutDelta.Affinity *= Weight;
	OutDelta.Fear *= Weight;
	OutDelta.Familiarity *= Weight;
	OutDelta.Reputation *= Weight;
	return true;
}

float FZLSocialRelationshipStore::SourceWeight(const EZLSocialPerceptionChannel Channel, const float Confidence)
{
	const float BoundedConfidence = FMath::Clamp(Confidence, 0.0f, 1.0f);
	if (Channel == EZLSocialPerceptionChannel::Direct) { return 1.0f; }
	if (Channel == EZLSocialPerceptionChannel::Visual) { return 0.75f * BoundedConfidence; }
	if (Channel == EZLSocialPerceptionChannel::Auditory) { return 0.45f * BoundedConfidence; }
	if (Channel == EZLSocialPerceptionChannel::Social) { return 0.6f * BoundedConfidence; }
	return 0.0f;
}
