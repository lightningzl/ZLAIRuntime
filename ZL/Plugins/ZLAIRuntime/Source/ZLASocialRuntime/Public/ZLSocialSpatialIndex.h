#pragma once

#include "CoreMinimal.h"
#include "ZLSocialTypes.h"

struct ZLASOCIALRUNTIME_API FZLSocialSpatialQueryStats
{
	int32 RegisteredAgents = 0;
	int32 CellsVisited = 0;
	int32 CandidatesExamined = 0;
	int32 ResultsReturned = 0;
};

class ZLASOCIALRUNTIME_API FZLSocialSpatialIndex
{
public:
	explicit FZLSocialSpatialIndex(float InCellSize = 1000.0f);

	bool RegisterAgent(const FZLSocialAgentProfile& Profile);
	bool UpdateAgentPosition(FName AgentId, const FVector& NewPosition);
	bool UnregisterAgent(FName AgentId);
	void Reset();

	void QueryRadius(const FVector& Center, float Radius, TArray<FZLSocialAgentProfile>& OutAgents, FZLSocialSpatialQueryStats* OutStats = nullptr) const;
	const FZLSocialAgentProfile* FindAgent(FName AgentId) const;
	int32 Num() const { return Agents.Num(); }
	float GetCellSize() const { return CellSize; }

private:
	FIntPoint ToCell(const FVector& Position) const;
	void AddToCell(FName AgentId, const FIntPoint& Cell);
	void RemoveFromCell(FName AgentId, const FIntPoint& Cell);

	float CellSize;
	TMap<FName, FZLSocialAgentProfile> Agents;
	TMap<FName, FIntPoint> AgentCells;
	TMap<FIntPoint, TSet<FName>> Cells;
};
