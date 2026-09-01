#include "ZLSocialObservation.h"

namespace
{
constexpr int32 MaxSpeechCodeUnits = 1024;
constexpr float MinLifetimeSeconds = 0.05f;
constexpr float MaxLifetimeSeconds = 10.0f;

FVector PlanarNormal(const FVector& Vector)
{
	return FVector(Vector.X, Vector.Y, 0.0f).GetSafeNormal();
}

bool ValidateCommon(const FGuid& EventId, const FName SourceId, const FVector& Position, const FVector& Forward, const double TimestampSeconds, const float LifetimeSeconds, const double NowSeconds, EZLSocialObservationFilterReason* OutReason)
{
	auto Reject = [OutReason](const EZLSocialObservationFilterReason Reason)
	{
		if (OutReason != nullptr) { *OutReason = Reason; }
		return false;
	};
	if (!EventId.IsValid() || SourceId.IsNone() || Position.ContainsNaN() || Forward.ContainsNaN() || PlanarNormal(Forward).IsNearlyZero() || TimestampSeconds < 0.0 || !FMath::IsFinite(TimestampSeconds) || !FMath::IsFinite(LifetimeSeconds) || LifetimeSeconds < MinLifetimeSeconds || LifetimeSeconds > MaxLifetimeSeconds)
	{
		return Reject(EZLSocialObservationFilterReason::InvalidEvent);
	}
	if (NowSeconds < TimestampSeconds || NowSeconds > TimestampSeconds + LifetimeSeconds)
	{
		return Reject(EZLSocialObservationFilterReason::Expired);
	}
	if (OutReason != nullptr) { *OutReason = EZLSocialObservationFilterReason::None; }
	return true;
}
}

bool FZLSocialSpeechEvent::IsValid(const double NowSeconds, EZLSocialObservationFilterReason* OutReason) const
{
	if (!ValidateCommon(EventId, SpeakerId, Position, Forward, TimestampSeconds, LifetimeSeconds, NowSeconds, OutReason))
	{
		return false;
	}
	const FString Trimmed = Text.TrimStartAndEnd();
	if (Trimmed.IsEmpty() || Text.Len() > MaxSpeechCodeUnits || (Mode == EZLSocialSpeechMode::InEar && ExplicitTargetId.IsNone()))
	{
		if (OutReason != nullptr) { *OutReason = EZLSocialObservationFilterReason::InvalidEvent; }
		return false;
	}
	return true;
}

bool FZLSocialActionEvent::IsValid(const double NowSeconds, EZLSocialObservationFilterReason* OutReason) const
{
	if (!ValidateCommon(EventId, ActorId, Position, Forward, TimestampSeconds, LifetimeSeconds, NowSeconds, OutReason))
	{
		return false;
	}
	if (Action != EZLSocialActionType::Stop && TargetId.IsNone())
	{
		if (OutReason != nullptr) { *OutReason = EZLSocialObservationFilterReason::InvalidEvent; }
		return false;
	}
	return true;
}

bool FZLSocialObserver::IsValid() const
{
	return !AgentId.IsNone() && !Position.ContainsNaN() && !Forward.ContainsNaN() && !PlanarNormal(Forward).IsNearlyZero();
}

void FZLSocialObservationSettings::Clamp()
{
	VisualRange = FMath::Clamp(VisualRange, 1.0f, 100000.0f);
	HorizontalFieldOfViewDegrees = FMath::Clamp(HorizontalFieldOfViewDegrees, 1.0f, 360.0f);
	WhisperRange = FMath::Clamp(WhisperRange, 1.0f, 100000.0f);
	TalkRange = FMath::Clamp(TalkRange, 1.0f, 100000.0f);
	ShoutRange = FMath::Clamp(ShoutRange, 1.0f, 100000.0f);
	InEarRange = FMath::Clamp(InEarRange, 1.0f, 100000.0f);
	ClearHearingThreshold = FMath::Clamp(ClearHearingThreshold, 0.0f, 1.0f);
}

float FZLSocialObservationSettings::HearingRange(const EZLSocialSpeechMode Mode) const
{
	switch (Mode)
	{
	case EZLSocialSpeechMode::Whisper: return WhisperRange;
	case EZLSocialSpeechMode::Shout: return ShoutRange;
	case EZLSocialSpeechMode::InEar: return InEarRange;
	default: return TalkRange;
	}
}

FZLSocialObservationEvaluator::FZLSocialObservationEvaluator(FZLSocialObservationSettings InSettings)
	: Settings(InSettings)
{
	Settings.Clamp();
}

FZLSocialObservation FZLSocialObservationEvaluator::ObserveSpeech(const FZLSocialSpeechEvent& Event, const FZLSocialObserver& Observer, const double NowSeconds) const
{
	FZLSocialObservation Result;
	Result.EventId = Event.EventId;
	Result.ObserverId = Observer.AgentId;
	Result.SourceId = Event.SpeakerId;
	Result.Source = EZLSocialObservationSource::Speech;
	Result.SpeechMode = Event.Mode;
	Result.ExplicitTargetId = Event.ExplicitTargetId;
	Result.ObservedAtSeconds = NowSeconds;

	EZLSocialObservationFilterReason EventReason = EZLSocialObservationFilterReason::None;
	if (!Observer.IsValid() || !Event.IsValid(NowSeconds, &EventReason))
	{
		Result.VisualFilter = EventReason == EZLSocialObservationFilterReason::None ? EZLSocialObservationFilterReason::InvalidEvent : EventReason;
		Result.AuditoryFilter = Result.VisualFilter;
		return Result;
	}

	Result.bSaw = EvaluateVision(Event.Position, Observer, Result.VisualFilter, Result.Distance);
	Result.TargetJudgment = JudgeTarget(Event, Observer);
	if (!Observer.bCanHear)
	{
		Result.AuditoryFilter = EZLSocialObservationFilterReason::CannotHear;
		return Result;
	}
	if (Event.Mode == EZLSocialSpeechMode::InEar && Event.ExplicitTargetId != Observer.AgentId)
	{
		Result.AuditoryFilter = EZLSocialObservationFilterReason::NotExplicitInEarTarget;
		return Result;
	}

	const float Range = Settings.HearingRange(Event.Mode);
	const float Distance = FVector::Dist2D(Event.Position, Observer.Position);
	Result.Distance = Distance;
	if (Distance > Range)
	{
		Result.AuditoryFilter = EZLSocialObservationFilterReason::OutsideHearingRange;
		return Result;
	}
	Result.bHeard = true;
	Result.HearingStrength = FMath::Clamp(1.0f - Distance / Range, 0.0f, 1.0f);
	if (FMath::IsNearlyEqual(Distance, Range))
	{
		Result.HearingStrength = 0.01f;
	}
	Result.bHeardClearly = Result.HearingStrength >= Settings.ClearHearingThreshold;
	return Result;
}

FZLSocialObservation FZLSocialObservationEvaluator::ObserveAction(const FZLSocialActionEvent& Event, const FZLSocialObserver& Observer, const double NowSeconds) const
{
	FZLSocialObservation Result;
	Result.EventId = Event.EventId;
	Result.ObserverId = Observer.AgentId;
	Result.SourceId = Event.ActorId;
	Result.Source = EZLSocialObservationSource::Action;
	Result.Action = Event.Action;
	Result.ActionPhase = Event.Phase;
	Result.ExplicitTargetId = Event.TargetId;
	Result.ObservedAtSeconds = NowSeconds;
	EZLSocialObservationFilterReason EventReason = EZLSocialObservationFilterReason::None;
	if (!Observer.IsValid() || !Event.IsValid(NowSeconds, &EventReason))
	{
		Result.VisualFilter = EventReason == EZLSocialObservationFilterReason::None ? EZLSocialObservationFilterReason::InvalidEvent : EventReason;
		Result.AuditoryFilter = EZLSocialObservationFilterReason::CannotHear;
		return Result;
	}
	Result.bSaw = EvaluateVision(Event.Position, Observer, Result.VisualFilter, Result.Distance);
	Result.AuditoryFilter = EZLSocialObservationFilterReason::CannotHear;
	Result.TargetJudgment = Event.TargetId == Observer.AgentId ? EZLSocialTargetJudgment::ExplicitSelf : (Event.TargetId.IsNone() ? EZLSocialTargetJudgment::Unresolved : EZLSocialTargetJudgment::ExplicitOther);
	return Result;
}

bool FZLSocialObservationEvaluator::EvaluateVision(const FVector& EventPosition, const FZLSocialObserver& Observer, EZLSocialObservationFilterReason& OutReason, float& OutDistance) const
{
	OutDistance = FVector::Dist2D(EventPosition, Observer.Position);
	if (!Observer.bCanSee)
	{
		OutReason = EZLSocialObservationFilterReason::CannotSee;
		return false;
	}
	if (OutDistance > Settings.VisualRange)
	{
		OutReason = EZLSocialObservationFilterReason::OutsideVisualRange;
		return false;
	}
	if (OutDistance <= UE_KINDA_SMALL_NUMBER)
	{
		OutReason = EZLSocialObservationFilterReason::None;
		return true;
	}
	const FVector ToEvent = PlanarNormal(EventPosition - Observer.Position);
	const float Dot = FVector::DotProduct(PlanarNormal(Observer.Forward), ToEvent);
	const float MinimumDot = FMath::Cos(FMath::DegreesToRadians(Settings.HorizontalFieldOfViewDegrees * 0.5f));
	if (Dot + UE_KINDA_SMALL_NUMBER < MinimumDot)
	{
		OutReason = EZLSocialObservationFilterReason::OutsideFieldOfView;
		return false;
	}
	OutReason = EZLSocialObservationFilterReason::None;
	return true;
}

EZLSocialTargetJudgment FZLSocialObservationEvaluator::JudgeTarget(const FZLSocialSpeechEvent& Event, const FZLSocialObserver& Observer) const
{
	if (!Event.ExplicitTargetId.IsNone())
	{
		return Event.ExplicitTargetId == Observer.AgentId ? EZLSocialTargetJudgment::ExplicitSelf : EZLSocialTargetJudgment::ExplicitOther;
	}
	const FVector TowardObserver = PlanarNormal(Observer.Position - Event.Position);
	const float FacingDot = FVector::DotProduct(PlanarNormal(Event.Forward), TowardObserver);
	return FacingDot >= FMath::Cos(FMath::DegreesToRadians(30.0f)) ? EZLSocialTargetJudgment::Candidate : EZLSocialTargetJudgment::Unresolved;
}

FZLSocialObservationBuffer::FZLSocialObservationBuffer(const int32 InCapacity)
	: Capacity(FMath::Clamp(InCapacity, 1, 32))
{
	Items.Reserve(Capacity);
}

void FZLSocialObservationBuffer::Add(const FZLSocialObservation& Observation)
{
	if (Items.Num() == Capacity)
	{
		Items.RemoveAt(0, 1, EAllowShrinking::No);
	}
	Items.Add(Observation);
}

const FZLSocialObservation* FZLSocialObservationBuffer::Latest() const
{
	return Items.IsEmpty() ? nullptr : &Items.Last();
}
