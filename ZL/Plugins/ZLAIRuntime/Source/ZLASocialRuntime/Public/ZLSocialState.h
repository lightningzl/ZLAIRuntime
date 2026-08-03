#pragma once

#include "CoreMinimal.h"
#include "ZLSocialPerception.h"
#include "ZLSocialState.generated.h"

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialInstantState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Fear = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Anger = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Curiosity = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Alert = 0.0f;

	void Apply(const FZLSocialEvent& Event, const FZLSocialPerceptionResult& Perception, const FZLSocialPersonalityTraits& Personality);
	void Decay(float DeltaSeconds, float DecayPerSecond = 0.08f);
	void Clamp();
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialMemoryEntry
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FGuid EventId;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FGameplayTag EventType;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName SourceId;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName TargetId;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector Position = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) double TimestampSeconds = 0.0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Confidence = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float EmotionalArousal = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Importance = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EZLSocialPerceptionChannel SourceChannel = EZLSocialPerceptionChannel::None;
};

class ZLASOCIALRUNTIME_API FZLSocialShortMemory
{
public:
	explicit FZLSocialShortMemory(int32 InCapacity = 6);

	void Add(const FZLSocialMemoryEntry& Entry);
	void Reset();
	int32 Num() const { return Count; }
	int32 GetCapacity() const { return Entries.Num(); }
	TArray<FZLSocialMemoryEntry> GetChronological() const;
	const FZLSocialMemoryEntry* FindByEvent(FGuid EventId) const;

private:
	TArray<FZLSocialMemoryEntry> Entries;
	int32 NextWriteIndex = 0;
	int32 Count = 0;
};

class ZLASOCIALRUNTIME_API FZLSocialAgentState
{
public:
	explicit FZLSocialAgentState(int32 MemoryCapacity = 6) : ShortMemory(MemoryCapacity) {}

	void ApplyPerception(const FZLSocialEvent& Event, const FZLSocialPerceptionResult& Perception, const FZLSocialPersonalityTraits& Personality);
	void Decay(float DeltaSeconds) { Instant.Decay(DeltaSeconds); }

	FZLSocialInstantState Instant;
	FZLSocialShortMemory ShortMemory;
};
