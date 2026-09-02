#include "SocialSandbox/Decision/ZLSocialSandboxDecisionScheduler.h"

FZLSocialSandboxDecisionScheduler::FZLSocialSandboxDecisionScheduler(const double InCooldownSeconds)
	: CooldownSeconds(FMath::Clamp(InCooldownSeconds, 0.0, 10.0))
{
}

EZLSocialSandboxQueueResult FZLSocialSandboxDecisionScheduler::Queue(
	const FZLSocialSandboxScheduledDecision& Decision)
{
	if (Decision.IsAutomaticReplan() && AutomaticReplanCount >= MaxAutomaticReplans)
	{
		return EZLSocialSandboxQueueResult::AutomaticLimit;
	}
	if (!Decision.IsAutomaticReplan())
	{
		AutomaticReplanCount = 0;
	}
	const bool bReplacedPending = bHasPending;
	Pending = Decision;
	bHasPending = true;
	if (bReplacedPending)
	{
		CoalescedCount = FMath::Min(CoalescedCount + 1, 999);
		return EZLSocialSandboxQueueResult::Coalesced;
	}
	return EZLSocialSandboxQueueResult::Accepted;
}

bool FZLSocialSandboxDecisionScheduler::TakeReady(
	const double NowSeconds,
	FZLSocialSandboxScheduledDecision& OutDecision,
	double& OutDelaySeconds)
{
	OutDelaySeconds = 0.0;
	if (bInFlight || !bHasPending)
	{
		return false;
	}
	const double EarliestDispatch = LastDispatchSeconds + CooldownSeconds;
	if (NowSeconds < EarliestDispatch)
	{
		OutDelaySeconds = EarliestDispatch - NowSeconds;
		return false;
	}
	OutDecision = MoveTemp(Pending);
	Pending = FZLSocialSandboxScheduledDecision();
	bHasPending = false;
	return true;
}

void FZLSocialSandboxDecisionScheduler::MarkDispatched(
	const double NowSeconds,
	const FZLSocialSandboxScheduledDecision& Decision)
{
	bInFlight = true;
	LastDispatchSeconds = NowSeconds;
	if (Decision.IsAutomaticReplan())
	{
		AutomaticReplanCount = FMath::Min(AutomaticReplanCount + 1, MaxAutomaticReplans);
	}
}

void FZLSocialSandboxDecisionScheduler::Reset()
{
	LastDispatchSeconds = -DBL_MAX;
	Pending = FZLSocialSandboxScheduledDecision();
	CoalescedCount = 0;
	AutomaticReplanCount = 0;
	bHasPending = false;
	bInFlight = false;
}

const TCHAR* FZLSocialSandboxDecisionScheduler::ReasonName(
	const EZLSocialSandboxDecisionTriggerReason Reason)
{
	switch (Reason)
	{
	case EZLSocialSandboxDecisionTriggerReason::PlayerAction: return TEXT("玩家行为");
	case EZLSocialSandboxDecisionTriggerReason::DistanceNear: return TEXT("距离接近");
	case EZLSocialSandboxDecisionTriggerReason::DistanceFar: return TEXT("距离拉远");
	case EZLSocialSandboxDecisionTriggerReason::PlanCompleted: return TEXT("计划完成");
	case EZLSocialSandboxDecisionTriggerReason::PlanInvalidated: return TEXT("计划失效");
	case EZLSocialSandboxDecisionTriggerReason::Hit: return TEXT("受到攻击");
	default: return TEXT("说话");
	}
}
