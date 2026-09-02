#include "SocialSandbox/Decision/ZLSocialSandboxMultiNpcDecision.h"

bool FZLSocialSandboxMultiNpcDecision::RegisterNpc(const FName NpcId)
{
	if (NpcId.IsNone() || Schedulers.Contains(NpcId) || Schedulers.Num() >= MaxNpcs)
	{
		return false;
	}
	StableOrder.Add(NpcId);
	Schedulers.Add(NpcId, FZLSocialSandboxDecisionScheduler());
	return true;
}

EZLSocialSandboxQueueResult FZLSocialSandboxMultiNpcDecision::Queue(
	const FName NpcId,
	const FZLSocialSandboxScheduledDecision& Decision)
{
	FZLSocialSandboxDecisionScheduler* Scheduler = Schedulers.Find(NpcId);
	return Scheduler == nullptr
		? EZLSocialSandboxQueueResult::AutomaticLimit
		: Scheduler->Queue(Decision);
}

bool FZLSocialSandboxMultiNpcDecision::TakeNext(
	const double NowSeconds,
	FZLSocialSandboxNpcDispatch& OutDispatch,
	double& OutDelaySeconds)
{
	OutDelaySeconds = 0.0;
	if (InFlightCount >= MaxInFlight || StableOrder.IsEmpty())
	{
		return false;
	}
	double ShortestDelay = DBL_MAX;
	for (int32 Offset = 0; Offset < StableOrder.Num(); ++Offset)
	{
		const int32 Index = (NextIndex + Offset) % StableOrder.Num();
		const FName NpcId = StableOrder[Index];
		FZLSocialSandboxDecisionScheduler* Scheduler = Schedulers.Find(NpcId);
		if (Scheduler == nullptr || Scheduler->IsInFlight())
		{
			continue;
		}
		FZLSocialSandboxScheduledDecision Decision;
		double DelaySeconds = 0.0;
		if (Scheduler->TakeReady(NowSeconds, Decision, DelaySeconds))
		{
			Scheduler->MarkDispatched(NowSeconds, Decision);
			++InFlightCount;
			NextIndex = (Index + 1) % StableOrder.Num();
			OutDispatch.NpcId = NpcId;
			OutDispatch.Decision = MoveTemp(Decision);
			return true;
		}
		if (Scheduler->HasPending() && DelaySeconds > 0.0)
		{
			ShortestDelay = FMath::Min(ShortestDelay, DelaySeconds);
		}
	}
	if (ShortestDelay < DBL_MAX)
	{
		OutDelaySeconds = ShortestDelay;
	}
	return false;
}

void FZLSocialSandboxMultiNpcDecision::MarkCompleted(const FName NpcId)
{
	FZLSocialSandboxDecisionScheduler* Scheduler = Schedulers.Find(NpcId);
	if (Scheduler != nullptr && Scheduler->IsInFlight())
	{
		Scheduler->MarkCompleted();
		InFlightCount = FMath::Max(0, InFlightCount - 1);
	}
}

void FZLSocialSandboxMultiNpcDecision::Reset()
{
	for (TPair<FName, FZLSocialSandboxDecisionScheduler>& Pair : Schedulers)
	{
		Pair.Value.Reset();
	}
	NextIndex = 0;
	InFlightCount = 0;
}

bool FZLSocialSandboxMultiNpcDecision::IsInFlight(const FName NpcId) const
{
	const FZLSocialSandboxDecisionScheduler* Scheduler = Schedulers.Find(NpcId);
	return Scheduler != nullptr && Scheduler->IsInFlight();
}

bool FZLSocialSandboxMultiNpcDecision::HasPending(const FName NpcId) const
{
	const FZLSocialSandboxDecisionScheduler* Scheduler = Schedulers.Find(NpcId);
	return Scheduler != nullptr && Scheduler->HasPending();
}

int32 FZLSocialSandboxMultiNpcDecision::GetCoalescedCount(const FName NpcId) const
{
	const FZLSocialSandboxDecisionScheduler* Scheduler = Schedulers.Find(NpcId);
	return Scheduler == nullptr ? 0 : Scheduler->GetCoalescedCount();
}

int32 FZLSocialSandboxMultiNpcDecision::GetAutomaticReplanCount(const FName NpcId) const
{
	const FZLSocialSandboxDecisionScheduler* Scheduler = Schedulers.Find(NpcId);
	return Scheduler == nullptr ? 0 : Scheduler->GetAutomaticReplanCount();
}
