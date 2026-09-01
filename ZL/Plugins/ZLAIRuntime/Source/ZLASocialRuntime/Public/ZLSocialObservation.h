#pragma once

#include "CoreMinimal.h"
#include "ZLSocialObservation.generated.h"

UENUM(BlueprintType)
enum class EZLSocialSpeechMode : uint8
{
	Whisper,
	Talk,
	Shout,
	InEar
};

UENUM(BlueprintType)
enum class EZLSocialActionType : uint8
{
	Face,
	Approach,
	MoveAway,
	Stop
};

UENUM(BlueprintType)
enum class EZLSocialActionPhase : uint8
{
	Started,
	Completed
};

UENUM(BlueprintType)
enum class EZLSocialObservationSource : uint8
{
	Speech,
	Action
};

UENUM(BlueprintType)
enum class EZLSocialTargetJudgment : uint8
{
	Unresolved,
	Candidate,
	ExplicitSelf,
	ExplicitOther
};

UENUM(BlueprintType)
enum class EZLSocialObservationFilterReason : uint8
{
	None,
	InvalidEvent,
	Expired,
	CannotSee,
	CannotHear,
	OutsideVisualRange,
	OutsideFieldOfView,
	OutsideHearingRange,
	NotExplicitInEarTarget
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialSpeechEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FGuid EventId;
	UPROPERTY(BlueprintReadOnly) FName SpeakerId;
	UPROPERTY(BlueprintReadOnly) FString Text;
	UPROPERTY(BlueprintReadOnly) EZLSocialSpeechMode Mode = EZLSocialSpeechMode::Talk;
	UPROPERTY(BlueprintReadOnly) FName ExplicitTargetId;
	UPROPERTY(BlueprintReadOnly) FVector Position = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly) FVector Forward = FVector::ForwardVector;
	UPROPERTY(BlueprintReadOnly) double TimestampSeconds = 0.0;
	UPROPERTY(BlueprintReadOnly) float LifetimeSeconds = 4.0f;

	bool IsValid(double NowSeconds, EZLSocialObservationFilterReason* OutReason = nullptr) const;
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialActionEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FGuid EventId;
	UPROPERTY(BlueprintReadOnly) EZLSocialActionType Action = EZLSocialActionType::Stop;
	UPROPERTY(BlueprintReadOnly) EZLSocialActionPhase Phase = EZLSocialActionPhase::Started;
	UPROPERTY(BlueprintReadOnly) FName ActorId;
	UPROPERTY(BlueprintReadOnly) FName TargetId;
	UPROPERTY(BlueprintReadOnly) FVector Position = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly) FVector Forward = FVector::ForwardVector;
	UPROPERTY(BlueprintReadOnly) double TimestampSeconds = 0.0;
	UPROPERTY(BlueprintReadOnly) float LifetimeSeconds = 4.0f;

	bool IsValid(double NowSeconds, EZLSocialObservationFilterReason* OutReason = nullptr) const;
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialObserver
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName AgentId;
	UPROPERTY(BlueprintReadOnly) FVector Position = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly) FVector Forward = FVector::ForwardVector;
	UPROPERTY(BlueprintReadOnly) bool bCanSee = true;
	UPROPERTY(BlueprintReadOnly) bool bCanHear = true;

	bool IsValid() const;
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialObservationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1.0")) float VisualRange = 1500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1.0", ClampMax="360.0")) float HorizontalFieldOfViewDegrees = 120.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1.0")) float WhisperRange = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1.0")) float TalkRange = 800.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1.0")) float ShoutRange = 2500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1.0")) float InEarRange = 150.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0")) float ClearHearingThreshold = 0.5f;

	void Clamp();
	float HearingRange(EZLSocialSpeechMode Mode) const;
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialObservation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FGuid EventId;
	UPROPERTY(BlueprintReadOnly) FName ObserverId;
	UPROPERTY(BlueprintReadOnly) FName SourceId;
	UPROPERTY(BlueprintReadOnly) EZLSocialObservationSource Source = EZLSocialObservationSource::Speech;
	UPROPERTY(BlueprintReadOnly) EZLSocialSpeechMode SpeechMode = EZLSocialSpeechMode::Talk;
	UPROPERTY(BlueprintReadOnly) EZLSocialActionType Action = EZLSocialActionType::Stop;
	UPROPERTY(BlueprintReadOnly) EZLSocialActionPhase ActionPhase = EZLSocialActionPhase::Started;
	UPROPERTY(BlueprintReadOnly) FName ExplicitTargetId;
	UPROPERTY(BlueprintReadOnly) EZLSocialTargetJudgment TargetJudgment = EZLSocialTargetJudgment::Unresolved;
	UPROPERTY(BlueprintReadOnly) bool bSaw = false;
	UPROPERTY(BlueprintReadOnly) bool bHeard = false;
	UPROPERTY(BlueprintReadOnly) bool bHeardClearly = false;
	UPROPERTY(BlueprintReadOnly) float HearingStrength = 0.0f;
	UPROPERTY(BlueprintReadOnly) float Distance = 0.0f;
	UPROPERTY(BlueprintReadOnly) EZLSocialObservationFilterReason VisualFilter = EZLSocialObservationFilterReason::None;
	UPROPERTY(BlueprintReadOnly) EZLSocialObservationFilterReason AuditoryFilter = EZLSocialObservationFilterReason::None;
	UPROPERTY(BlueprintReadOnly) double ObservedAtSeconds = 0.0;
};

class ZLASOCIALRUNTIME_API FZLSocialObservationEvaluator
{
public:
	explicit FZLSocialObservationEvaluator(FZLSocialObservationSettings InSettings = FZLSocialObservationSettings());

	FZLSocialObservation ObserveSpeech(const FZLSocialSpeechEvent& Event, const FZLSocialObserver& Observer, double NowSeconds) const;
	FZLSocialObservation ObserveAction(const FZLSocialActionEvent& Event, const FZLSocialObserver& Observer, double NowSeconds) const;

private:
	bool EvaluateVision(const FVector& EventPosition, const FZLSocialObserver& Observer, EZLSocialObservationFilterReason& OutReason, float& OutDistance) const;
	EZLSocialTargetJudgment JudgeTarget(const FZLSocialSpeechEvent& Event, const FZLSocialObserver& Observer) const;

	FZLSocialObservationSettings Settings;
};

class ZLASOCIALRUNTIME_API FZLSocialObservationBuffer
{
public:
	explicit FZLSocialObservationBuffer(int32 InCapacity = 16);
	void Add(const FZLSocialObservation& Observation);
	void Reset() { Items.Reset(); }
	const FZLSocialObservation* Latest() const;
	const TArray<FZLSocialObservation>& GetItems() const { return Items; }
	int32 GetCapacity() const { return Capacity; }

private:
	int32 Capacity = 16;
	TArray<FZLSocialObservation> Items;
};
