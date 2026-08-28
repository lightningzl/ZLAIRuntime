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
	Event.RootEventId = Event.EventId;
	Event.Type = Type;
	Event.SourceId = SourceId;
	Event.TargetId = TargetId;
	Event.Position = Position;
	Event.CreatedAtSeconds = NowSeconds;
	Event.ChainBudget = ZLSocialEventLimits::DefaultChainBudget;
	Event.Confidence = 1.0f;
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
	for (auto Iterator = RootExpiryByRoot.CreateIterator(); Iterator; ++Iterator)
	{
		if (Iterator.Value() < NowSeconds)
		{
			DeliveredAgentsByRoot.Remove(Iterator.Key());
			Iterator.RemoveCurrent();
		}
	}
	if (!RootExpiryByRoot.Contains(Event.RootEventId))
	{
		if (RootExpiryByRoot.Num() >= ZLSocialRuntimeLimits::MaxTrackedRoots) { return false; }
		RootExpiryByRoot.Add(Event.RootEventId, Event.ExpiresAtSeconds);
	}

	TArray<FZLSocialAgentProfile> QueriedAgents;
	SpatialIndex.QueryRadius(Event.Position, Event.Radius, QueriedAgents, &OutResult.SpatialStats);
	if (Event.HasChannel(EZLSocialPerceptionChannel::Direct) && !Event.TargetId.IsNone())
	{
		const FZLSocialAgentProfile* DirectTarget = SpatialIndex.FindAgent(Event.TargetId);
		if (DirectTarget != nullptr && !QueriedAgents.ContainsByPredicate([&Event](const FZLSocialAgentProfile& Agent) { return Agent.AgentId == Event.TargetId; }))
		{
			QueriedAgents.Add(*DirectTarget);
		}
	}
	if (Event.HasChannel(EZLSocialPerceptionChannel::Social) && !Event.SocialReceiverId.IsNone())
	{
		const FZLSocialAgentProfile* SocialReceiver = SpatialIndex.FindAgent(Event.SocialReceiverId);
		if (SocialReceiver != nullptr && !QueriedAgents.ContainsByPredicate([&Event](const FZLSocialAgentProfile& Agent) { return Agent.AgentId == Event.SocialReceiverId; }))
		{
			QueriedAgents.Add(*SocialReceiver);
		}
	}
	TSet<FName>& DeliveredAgents = DeliveredAgentsByRoot.FindOrAdd(Event.RootEventId);
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
	DeliveredAgentsByRoot.Reset();
	RootExpiryByRoot.Reset();
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
