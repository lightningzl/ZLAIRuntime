#pragma once

#include "CoreMinimal.h"
#include "ZLSocialSpatialIndex.h"

struct ZLASOCIALRUNTIME_API FZLSocialEventRouteResult
{
	TArray<FZLSocialAgentProfile> Candidates;
	FZLSocialSpatialQueryStats SpatialStats;
	int32 DuplicateCount = 0;
};

class ZLASOCIALRUNTIME_API FZLSocialEventRouter
{
public:
	explicit FZLSocialEventRouter(float CellSize = 1000.0f);

	bool RegisterAgent(const FZLSocialAgentProfile& Profile) { return SpatialIndex.RegisterAgent(Profile); }
	bool UpdateAgentPosition(FName AgentId, const FVector& Position) { return SpatialIndex.UpdateAgentPosition(AgentId, Position); }
	bool UnregisterAgent(FName AgentId) { return SpatialIndex.UnregisterAgent(AgentId); }

	bool CreateEvent(FGameplayTag Type, FName SourceId, FName TargetId, const FVector& Position, double NowSeconds, FZLSocialEvent& OutEvent) const;
	bool RouteEvent(const FZLSocialEvent& Event, double NowSeconds, FZLSocialEventRouteResult& OutResult);
	void Reset();
	int32 GetRegisteredAgentCount() const { return SpatialIndex.Num(); }
	int32 GetTrackedRootCount() const { return DeliveredAgentsByRoot.Num(); }

private:
	static bool ApplyDefaults(FGameplayTag Type, FZLSocialEvent& Event);

	FZLSocialSpatialIndex SpatialIndex;
	TMap<FGuid, TSet<FName>> DeliveredAgentsByRoot;
	TMap<FGuid, double> RootExpiryByRoot;
};
