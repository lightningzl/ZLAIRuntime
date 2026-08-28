#include "ZLSocialState.h"

#include "ZLSocialTags.h"

void FZLSocialInstantState::Apply(const FZLSocialEvent& Event, const FZLSocialPerceptionResult& Perception, const FZLSocialPersonalityTraits& Personality)
{
	if (!Perception.bPerceived) { return; }
	const float Intensity = Perception.EffectiveIntensity;
	const bool bExtreme = Event.Type == ZLSocialTags::Event_Gunshot || Event.Type == ZLSocialTags::Event_Kill;
	const bool bHostile = bExtreme || Event.Type == ZLSocialTags::Event_Punch || Event.Type == ZLSocialTags::Event_Steal;
	const bool bHelpful = Event.Type == ZLSocialTags::Event_Help;

	Alert += Intensity * (bExtreme ? 1.0f : 0.55f);
	Fear += bHostile ? Intensity * (0.35f + Personality.FearSensitivity * 0.65f) : 0.0f;
	Anger += bHostile ? Intensity * (0.2f + Personality.Justice * 0.4f + Personality.Aggression * 0.4f) : 0.0f;
	Curiosity += Intensity * Personality.Curiosity * (bExtreme ? 0.1f : 0.7f);
	if (bHelpful)
	{
		Fear -= Intensity * 0.25f;
		Curiosity += Intensity * Personality.Social * 0.3f;
	}
	Clamp();
}

void FZLSocialInstantState::Decay(const float DeltaSeconds, const float DecayPerSecond)
{
	const float Amount = FMath::Max(DeltaSeconds, 0.0f) * FMath::Max(DecayPerSecond, 0.0f);
	Fear = FMath::Max(0.0f, Fear - Amount);
	Anger = FMath::Max(0.0f, Anger - Amount);
	Curiosity = FMath::Max(0.0f, Curiosity - Amount);
	Alert = FMath::Max(0.0f, Alert - Amount);
}

void FZLSocialInstantState::Clamp()
{
	Fear = FMath::Clamp(Fear, 0.0f, 1.0f);
	Anger = FMath::Clamp(Anger, 0.0f, 1.0f);
	Curiosity = FMath::Clamp(Curiosity, 0.0f, 1.0f);
	Alert = FMath::Clamp(Alert, 0.0f, 1.0f);
}

FZLSocialShortMemory::FZLSocialShortMemory(const int32 InCapacity)
{
	Entries.SetNum(FMath::Max(InCapacity, 1));
}

void FZLSocialShortMemory::Add(const FZLSocialMemoryEntry& Entry)
{
	Entries[NextWriteIndex] = Entry;
	NextWriteIndex = (NextWriteIndex + 1) % Entries.Num();
	Count = FMath::Min(Count + 1, Entries.Num());
}

void FZLSocialShortMemory::Reset()
{
	NextWriteIndex = 0;
	Count = 0;
}

TArray<FZLSocialMemoryEntry> FZLSocialShortMemory::GetChronological() const
{
	TArray<FZLSocialMemoryEntry> Result;
	Result.Reserve(Count);
	const int32 StartIndex = Count == Entries.Num() ? NextWriteIndex : 0;
	for (int32 Offset = 0; Offset < Count; ++Offset)
	{
		Result.Add(Entries[(StartIndex + Offset) % Entries.Num()]);
	}
	return Result;
}

const FZLSocialMemoryEntry* FZLSocialShortMemory::FindByEvent(const FGuid EventId) const
{
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const FZLSocialMemoryEntry& Entry = Entries[Index];
		if (Entry.EventId == EventId) { return &Entry; }
	}
	return nullptr;
}

FZLSocialLongMemory::FZLSocialLongMemory(const int32 InCapacity, const float InPromotionThreshold, const float InDecayRate)
	: Capacity(FMath::Clamp(InCapacity, 0, ZLSocialMemoryLimits::MaxLongCapacity))
	, PromotionThreshold(FMath::Clamp(InPromotionThreshold, 0.0f, 1.0f))
	, DecayRate(FMath::Max(InDecayRate, 0.0f))
{
	Entries.Reserve(Capacity);
}

bool FZLSocialLongMemory::AddPromoted(const FZLSocialMemoryEntry& Entry, const double NowSeconds)
{
	if (Capacity <= 0 || (!Entry.bAnchored && Entry.Importance < PromotionThreshold))
	{
		return false;
	}
	if (Entries.ContainsByPredicate([&Entry](const FZLSocialMemoryEntry& Existing) { return Existing.RootEventId == Entry.RootEventId; }))
	{
		return false;
	}
	if (Entries.Num() >= Capacity)
	{
		int32 EvictionIndex = 0;
		for (int32 Index = 1; Index < Entries.Num(); ++Index)
		{
			const float Candidate = EffectiveImportance(Entries[Index], NowSeconds);
			const float Current = EffectiveImportance(Entries[EvictionIndex], NowSeconds);
			if (Candidate < Current || (FMath::IsNearlyEqual(Candidate, Current) && Entries[Index].TimestampSeconds < Entries[EvictionIndex].TimestampSeconds))
			{
				EvictionIndex = Index;
			}
		}
		Entries.RemoveAt(EvictionIndex);
	}
	Entries.Add(Entry);
	return true;
}

TArray<FZLSocialMemoryEntry> FZLSocialLongMemory::Retrieve(const FZLSocialLongMemoryQuery& Query, const double NowSeconds) const
{
	struct FScoredEntry
	{
		const FZLSocialMemoryEntry* Entry = nullptr;
		float Score = 0.0f;
	};
	TArray<FScoredEntry> Matches;
	for (const FZLSocialMemoryEntry& Entry : Entries)
	{
		if (Query.EventType.IsValid() && Entry.EventType != Query.EventType) { continue; }
		if (!Query.SubjectId.IsNone() && Entry.SourceId != Query.SubjectId && Entry.TargetId != Query.SubjectId) { continue; }
		if (!Query.FactionId.IsNone() && Entry.SubjectFactionId != Query.FactionId) { continue; }
		if (Entry.TimestampSeconds < Query.MinTimestampSeconds || Entry.TimestampSeconds > Query.MaxTimestampSeconds) { continue; }
		if (Query.LocationRadius >= 0.0f && FVector::DistSquared2D(Entry.Position, Query.Location) > FMath::Square(Query.LocationRadius)) { continue; }
		const float QueryRelevance = (!Query.SubjectId.IsNone() || !Query.FactionId.IsNone() || Query.EventType.IsValid()) ? 0.15f : 0.0f;
		const float EmotionalMatch = Query.PreferredEmotionalArousal >= 0.0f ? 0.1f * (1.0f - FMath::Abs(Entry.EmotionalArousal - Query.PreferredEmotionalArousal)) : 0.0f;
		Matches.Add({&Entry, EffectiveImportance(Entry, NowSeconds) + QueryRelevance + EmotionalMatch});
	}
	Matches.Sort([](const FScoredEntry& A, const FScoredEntry& B)
	{
		if (!FMath::IsNearlyEqual(A.Score, B.Score)) { return A.Score > B.Score; }
		if (!FMath::IsNearlyEqual(A.Entry->TimestampSeconds, B.Entry->TimestampSeconds)) { return A.Entry->TimestampSeconds > B.Entry->TimestampSeconds; }
		return A.Entry->EventId.ToString().Compare(B.Entry->EventId.ToString(), ESearchCase::CaseSensitive) < 0;
	});

	TArray<FZLSocialMemoryEntry> Result;
	const int32 Count = FMath::Min(Matches.Num(), FMath::Clamp(Query.TopK, 0, ZLSocialMemoryLimits::MaxTopK));
	Result.Reserve(Count);
	for (int32 Index = 0; Index < Count; ++Index) { Result.Add(*Matches[Index].Entry); }
	return Result;
}

float FZLSocialLongMemory::EffectiveImportance(const FZLSocialMemoryEntry& Entry, const double NowSeconds) const
{
	const double Elapsed = FMath::Max(NowSeconds - Entry.TimestampSeconds, 0.0);
	return Entry.Importance * FMath::Exp(-DecayRate * static_cast<float>(Elapsed));
}

void FZLSocialAgentState::ApplyPerception(const FZLSocialEvent& Event, const FZLSocialPerceptionResult& Perception, const FZLSocialPersonalityTraits& Personality, const float RelationshipImpact, const FName SubjectFactionId)
{
	if (!Perception.bPerceived) { return; }
	Instant.Apply(Event, Perception, Personality);

	FZLSocialMemoryEntry Entry;
	Entry.EventId = Event.EventId;
	Entry.RootEventId = Event.RootEventId;
	Entry.CausationId = Event.CausationId;
	Entry.EventType = Event.Type;
	Entry.SourceId = Event.SourceId;
	Entry.TargetId = Event.TargetId;
	Entry.ReporterId = Event.ReporterId;
	Entry.SubjectFactionId = SubjectFactionId;
	Entry.Position = Event.Position;
	Entry.TimestampSeconds = Event.CreatedAtSeconds;
	Entry.Confidence = Perception.Channel == EZLSocialPerceptionChannel::Direct ? 1.0f : FMath::Clamp(Perception.EffectiveIntensity, 0.0f, 1.0f);
	Entry.EmotionalArousal = FMath::Max(Instant.Fear, Instant.Anger);
	Entry.RelationshipImpact = FMath::Clamp(RelationshipImpact, 0.0f, 1.0f);
	const float PersonalRelevance = Event.TargetId.IsNone() ? 0.5f : 1.0f;
	Entry.Importance = FMath::Clamp(0.35f * Event.Severity + 0.25f * PersonalRelevance + 0.15f + 0.15f * Entry.RelationshipImpact + 0.1f * Entry.EmotionalArousal, 0.0f, 1.0f);
	Entry.bAnchored = Event.bAnchored;
	Entry.SourceChannel = Perception.Channel;
	ShortMemory.Add(Entry);
	LongMemory.AddPromoted(Entry, Event.CreatedAtSeconds);
}
