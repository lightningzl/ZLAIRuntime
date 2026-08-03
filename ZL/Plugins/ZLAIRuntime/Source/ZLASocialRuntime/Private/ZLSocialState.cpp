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

void FZLSocialAgentState::ApplyPerception(const FZLSocialEvent& Event, const FZLSocialPerceptionResult& Perception, const FZLSocialPersonalityTraits& Personality)
{
	if (!Perception.bPerceived) { return; }
	Instant.Apply(Event, Perception, Personality);

	FZLSocialMemoryEntry Entry;
	Entry.EventId = Event.EventId;
	Entry.EventType = Event.Type;
	Entry.SourceId = Event.SourceId;
	Entry.TargetId = Event.TargetId;
	Entry.Position = Event.Position;
	Entry.TimestampSeconds = Event.CreatedAtSeconds;
	Entry.Confidence = Perception.Channel == EZLSocialPerceptionChannel::Direct ? 1.0f : Perception.EffectiveIntensity;
	Entry.EmotionalArousal = FMath::Max(Instant.Fear, Instant.Anger);
	Entry.Importance = FMath::Clamp(0.65f * Event.Severity + 0.25f * Entry.EmotionalArousal + 0.1f * Entry.Confidence, 0.0f, 1.0f);
	Entry.SourceChannel = Perception.Channel;
	ShortMemory.Add(Entry);
}
