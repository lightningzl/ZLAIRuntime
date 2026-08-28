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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FGuid RootEventId;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FGuid CausationId;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FGameplayTag EventType;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName SourceId;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName TargetId;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName ReporterId;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName SubjectFactionId;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector Position = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) double TimestampSeconds = 0.0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Confidence = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float EmotionalArousal = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Importance = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float RelationshipImpact = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bAnchored = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EZLSocialPerceptionChannel SourceChannel = EZLSocialPerceptionChannel::None;
};

namespace ZLSocialMemoryLimits
{
	inline constexpr int32 Level1ShortCapacity = 6;
	inline constexpr int32 ImportantShortCapacity = 16;
	inline constexpr int32 ImportantLongCapacity = 8;
	inline constexpr int32 MaxShortCapacity = 32;
	inline constexpr int32 MaxLongCapacity = 24;
	inline constexpr int32 MaxTopK = 8;
}

struct ZLASOCIALRUNTIME_API FZLSocialLongMemoryQuery
{
	FGameplayTag EventType;
	FName SubjectId;
	FName FactionId;
	FVector Location = FVector::ZeroVector;
	float LocationRadius = -1.0f;
	double MinTimestampSeconds = -TNumericLimits<double>::Max();
	double MaxTimestampSeconds = TNumericLimits<double>::Max();
	float PreferredEmotionalArousal = -1.0f;
	int32 TopK = 3;
};

class ZLASOCIALRUNTIME_API FZLSocialLongMemory
{
public:
	explicit FZLSocialLongMemory(int32 InCapacity = 0, float InPromotionThreshold = 0.65f, float InDecayRate = 0.001f);

	bool AddPromoted(const FZLSocialMemoryEntry& Entry, double NowSeconds);
	TArray<FZLSocialMemoryEntry> Retrieve(const FZLSocialLongMemoryQuery& Query, double NowSeconds) const;
	void Reset() { Entries.Reset(); }
	int32 Num() const { return Entries.Num(); }
	int32 GetCapacity() const { return Capacity; }
	const TArray<FZLSocialMemoryEntry>& GetEntries() const { return Entries; }

private:
	float EffectiveImportance(const FZLSocialMemoryEntry& Entry, double NowSeconds) const;

	int32 Capacity = 0;
	float PromotionThreshold = 0.65f;
	float DecayRate = 0.001f;
	TArray<FZLSocialMemoryEntry> Entries;
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
	explicit FZLSocialAgentState(int32 MemoryCapacity = ZLSocialMemoryLimits::Level1ShortCapacity, int32 LongMemoryCapacity = 0)
		: ShortMemory(FMath::Clamp(MemoryCapacity, 1, ZLSocialMemoryLimits::MaxShortCapacity))
		, LongMemory(FMath::Clamp(LongMemoryCapacity, 0, ZLSocialMemoryLimits::MaxLongCapacity)) {}

	void ApplyPerception(const FZLSocialEvent& Event, const FZLSocialPerceptionResult& Perception, const FZLSocialPersonalityTraits& Personality, float RelationshipImpact = 0.0f, FName SubjectFactionId = NAME_None);
	void Decay(float DeltaSeconds) { Instant.Decay(DeltaSeconds); }

	FZLSocialInstantState Instant;
	FZLSocialShortMemory ShortMemory;
	FZLSocialLongMemory LongMemory;
};
