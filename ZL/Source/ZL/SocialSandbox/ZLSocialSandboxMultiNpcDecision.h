#pragma once

#include "CoreMinimal.h"
#include "SocialSandbox/ZLSocialSandboxDecisionScheduler.h"

struct FZLSocialSandboxNpcDispatch
{
	FName NpcId;
	FZLSocialSandboxScheduledDecision Decision;
};

class FZLSocialSandboxMultiNpcDecision
{
public:
	static constexpr int32 MaxNpcs = 4;
	static constexpr int32 MaxInFlight = 2;

	bool RegisterNpc(FName NpcId);
	EZLSocialSandboxQueueResult Queue(FName NpcId, const FZLSocialSandboxScheduledDecision& Decision);
	bool TakeNext(double NowSeconds, FZLSocialSandboxNpcDispatch& OutDispatch, double& OutDelaySeconds);
	void MarkCompleted(FName NpcId);
	void Reset();

	bool IsRegistered(FName NpcId) const { return Schedulers.Contains(NpcId); }
	bool IsInFlight(FName NpcId) const;
	bool HasPending(FName NpcId) const;
	int32 GetInFlightCount() const { return InFlightCount; }
	int32 GetCoalescedCount(FName NpcId) const;
	int32 GetAutomaticReplanCount(FName NpcId) const;

private:
	TArray<FName> StableOrder;
	TMap<FName, FZLSocialSandboxDecisionScheduler> Schedulers;
	int32 NextIndex = 0;
	int32 InFlightCount = 0;
};
