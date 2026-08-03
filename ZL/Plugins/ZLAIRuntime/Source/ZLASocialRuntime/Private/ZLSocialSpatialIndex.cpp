#include "ZLSocialSpatialIndex.h"

FZLSocialSpatialIndex::FZLSocialSpatialIndex(const float InCellSize)
	: CellSize(FMath::Max(InCellSize, 1.0f))
{
}

bool FZLSocialSpatialIndex::RegisterAgent(const FZLSocialAgentProfile& Profile)
{
	if (!Profile.IsValid() || Agents.Contains(Profile.AgentId))
	{
		return false;
	}

	Agents.Add(Profile.AgentId, Profile);
	const FIntPoint Cell = ToCell(Profile.Position);
	AgentCells.Add(Profile.AgentId, Cell);
	AddToCell(Profile.AgentId, Cell);
	return true;
}

bool FZLSocialSpatialIndex::UpdateAgentPosition(const FName AgentId, const FVector& NewPosition)
{
	FZLSocialAgentProfile* Profile = Agents.Find(AgentId);
	FIntPoint* OldCell = AgentCells.Find(AgentId);
	if (Profile == nullptr || OldCell == nullptr)
	{
		return false;
	}

	const FIntPoint NewCell = ToCell(NewPosition);
	if (NewCell != *OldCell)
	{
		RemoveFromCell(AgentId, *OldCell);
		AddToCell(AgentId, NewCell);
		*OldCell = NewCell;
	}
	Profile->Position = NewPosition;
	return true;
}

bool FZLSocialSpatialIndex::UnregisterAgent(const FName AgentId)
{
	const FIntPoint* Cell = AgentCells.Find(AgentId);
	if (!Agents.Contains(AgentId) || Cell == nullptr)
	{
		return false;
	}

	RemoveFromCell(AgentId, *Cell);
	AgentCells.Remove(AgentId);
	Agents.Remove(AgentId);
	return true;
}

void FZLSocialSpatialIndex::Reset()
{
	Agents.Reset();
	AgentCells.Reset();
	Cells.Reset();
}

void FZLSocialSpatialIndex::QueryRadius(const FVector& Center, const float Radius, TArray<FZLSocialAgentProfile>& OutAgents, FZLSocialSpatialQueryStats* OutStats) const
{
	OutAgents.Reset();
	FZLSocialSpatialQueryStats Stats;
	Stats.RegisteredAgents = Agents.Num();
	if (Radius < 0.0f)
	{
		if (OutStats != nullptr) { *OutStats = Stats; }
		return;
	}

	const FIntPoint MinCell = ToCell(Center - FVector(Radius, Radius, 0.0f));
	const FIntPoint MaxCell = ToCell(Center + FVector(Radius, Radius, 0.0f));
	const float RadiusSquared = FMath::Square(Radius);

	for (int32 X = MinCell.X; X <= MaxCell.X; ++X)
	{
		for (int32 Y = MinCell.Y; Y <= MaxCell.Y; ++Y)
		{
			++Stats.CellsVisited;
			const TSet<FName>* CellAgents = Cells.Find(FIntPoint(X, Y));
			if (CellAgents == nullptr) { continue; }

			for (const FName AgentId : *CellAgents)
			{
				++Stats.CandidatesExamined;
				const FZLSocialAgentProfile& Profile = Agents.FindChecked(AgentId);
				if (FVector::DistSquared2D(Profile.Position, Center) <= RadiusSquared)
				{
					OutAgents.Add(Profile);
				}
			}
		}
	}

	OutAgents.Sort([](const FZLSocialAgentProfile& A, const FZLSocialAgentProfile& B)
	{
		return A.AgentId.LexicalLess(B.AgentId);
	});
	Stats.ResultsReturned = OutAgents.Num();
	if (OutStats != nullptr) { *OutStats = Stats; }
}

const FZLSocialAgentProfile* FZLSocialSpatialIndex::FindAgent(const FName AgentId) const
{
	return Agents.Find(AgentId);
}

FIntPoint FZLSocialSpatialIndex::ToCell(const FVector& Position) const
{
	return FIntPoint(FMath::FloorToInt(Position.X / CellSize), FMath::FloorToInt(Position.Y / CellSize));
}

void FZLSocialSpatialIndex::AddToCell(const FName AgentId, const FIntPoint& Cell)
{
	Cells.FindOrAdd(Cell).Add(AgentId);
}

void FZLSocialSpatialIndex::RemoveFromCell(const FName AgentId, const FIntPoint& Cell)
{
	TSet<FName>* CellAgents = Cells.Find(Cell);
	if (CellAgents == nullptr) { return; }
	CellAgents->Remove(AgentId);
	if (CellAgents->IsEmpty()) { Cells.Remove(Cell); }
}
