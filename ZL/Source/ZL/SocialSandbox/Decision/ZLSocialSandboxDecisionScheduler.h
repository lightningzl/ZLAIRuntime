#pragma once

#include "CoreMinimal.h"
#include "ZLSocialObservation.h"

enum class EZLSocialSandboxDecisionTriggerReason : uint8
{
	Speech,
	PlayerAction,
	DistanceNear,
	DistanceFar,
	PlanCompleted,
	PlanInvalidated,
	Hit
};

struct FZLSocialSandboxScheduledDecision
{
	FZLSocialObservation Observation;
	FString SpeechContent;
	EZLSocialSandboxDecisionTriggerReason Reason = EZLSocialSandboxDecisionTriggerReason::Speech;

	bool IsAutomaticReplan() const
	{
		return Reason == EZLSocialSandboxDecisionTriggerReason::PlanCompleted
			|| Reason == EZLSocialSandboxDecisionTriggerReason::PlanInvalidated;
	}
};

enum class EZLSocialSandboxQueueResult : uint8
{
	Accepted,
	Coalesced,
	AutomaticLimit
};

class FZLSocialSandboxDecisionScheduler
{
public:
	static constexpr int32 MaxAutomaticReplans = 3;
	static constexpr double DefaultCooldownSeconds = 0.75;

	explicit FZLSocialSandboxDecisionScheduler(double InCooldownSeconds = DefaultCooldownSeconds);

	EZLSocialSandboxQueueResult Queue(const FZLSocialSandboxScheduledDecision& Decision);
	bool TakeReady(double NowSeconds, FZLSocialSandboxScheduledDecision& OutDecision, double& OutDelaySeconds);
	void MarkDispatched(double NowSeconds, const FZLSocialSandboxScheduledDecision& Decision);
	void MarkCompleted() { bInFlight = false; }
	void Reset();

	bool IsInFlight() const { return bInFlight; }
	bool HasPending() const { return bHasPending; }
	int32 GetCoalescedCount() const { return CoalescedCount; }
	int32 GetAutomaticReplanCount() const { return AutomaticReplanCount; }

	static const TCHAR* ReasonName(EZLSocialSandboxDecisionTriggerReason Reason);

private:
	double CooldownSeconds = DefaultCooldownSeconds;
	double LastDispatchSeconds = -DBL_MAX;
	FZLSocialSandboxScheduledDecision Pending;
	int32 CoalescedCount = 0;
	int32 AutomaticReplanCount = 0;
	bool bHasPending = false;
	bool bInFlight = false;
};
