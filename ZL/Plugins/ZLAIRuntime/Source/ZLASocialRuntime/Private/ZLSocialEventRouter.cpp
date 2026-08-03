#include "ZLSocialEventRouter.h"

#include "ZLSocialTags.h"

FZLSocialEventRouter::FZLSocialEventRouter(const float CellSize)
	: SpatialIndex(CellSize)
{
}

bool FZLSocialEventRouter::CreateEvent(const FGameplayTag Type, const FName SourceId, const FName TargetId, const FVector& Position, const double NowSeconds, FZLSocialEvent& OutEvent) const
{
	FZLSocialEvent Event;
	Event.EventId = FGuid::NewGuid();
	Event.Type = Type;
	Event.SourceId = SourceId;
	Event.TargetId = TargetId;
	Event.Position = Position;
	Event.CreatedAtSeconds = NowSeconds;
	if (!ApplyDefaults(Type, Event))
	{
		return false;
	}
	Event.ExpiresAtSeconds += NowSeconds;
	OutEvent = MoveTemp(Event);
	return true;
}

bool FZLSocialEventRouter::RouteEvent(const FZLSocialEvent& Event, const double NowSeconds, FZLSocialEventRouteResult& OutResult)
{
	OutResult = FZLSocialEventRouteResult();
	if (!Event.IsValid(NowSeconds))
	{
		return false;
	}

	TArray<FZLSocialAgentProfile> QueriedAgents;
	SpatialIndex.QueryRadius(Event.Position, Event.Radius, QueriedAgents, &OutResult.SpatialStats);
	TSet<FName>& DeliveredAgents = DeliveredAgentsByEvent.FindOrAdd(Event.EventId);
	for (const FZLSocialAgentProfile& Agent : QueriedAgents)
	{
		if (DeliveredAgents.Contains(Agent.AgentId))
		{
			++OutResult.DuplicateCount;
			continue;
		}
		DeliveredAgents.Add(Agent.AgentId);
		OutResult.Candidates.Add(Agent);
	}
	return true;
}

void FZLSocialEventRouter::Reset()
{
	SpatialIndex.Reset();
	DeliveredAgentsByEvent.Reset();
}

bool FZLSocialEventRouter::ApplyDefaults(const FGameplayTag Type, FZLSocialEvent& Event)
{
	Event.Channels = static_cast<int32>(EZLSocialPerceptionChannel::Visual) | static_cast<int32>(EZLSocialPerceptionChannel::Auditory);
	Event.ExpiresAtSeconds = 2.0;
	if (Type == ZLSocialTags::Event_Punch)
	{
		Event.Radius = 1000.0f; Event.Severity = 0.55f; Event.Noise = 0.45f;
	}
	else if (Type == ZLSocialTags::Event_Gunshot)
	{
		Event.Radius = 10000.0f; Event.Severity = 1.0f; Event.Noise = 1.0f;
	}
	else if (Type == ZLSocialTags::Event_Help)
	{
		Event.Radius = 1200.0f; Event.Severity = 0.25f; Event.Noise = 0.3f;
	}
	else
	{
		return false;
	}

	if (!Event.TargetId.IsNone())
	{
		Event.Channels |= static_cast<int32>(EZLSocialPerceptionChannel::Direct);
	}
	return true;
}
