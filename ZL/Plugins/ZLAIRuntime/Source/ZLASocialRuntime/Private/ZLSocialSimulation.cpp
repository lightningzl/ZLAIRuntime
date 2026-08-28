#include "ZLSocialSimulation.h"

#include "ZLSocialDebug.h"

FZLSocialSimulation::FZLSocialSimulation(const float CellSize)
	: Router(CellSize)
{
}

bool FZLSocialSimulation::RegisterAgent(const FZLSocialAgentProfile& Profile)
{
	if (!Router.RegisterAgent(Profile)) { return false; }
	Profiles.Add(Profile.AgentId, Profile);
	const int32 ShortCapacity = Profile.IsImportant()
		? FMath::Clamp(Profile.ShortMemoryCapacity > ZLSocialMemoryLimits::Level1ShortCapacity ? Profile.ShortMemoryCapacity : ZLSocialMemoryLimits::ImportantShortCapacity, 1, ZLSocialMemoryLimits::MaxShortCapacity)
		: ZLSocialMemoryLimits::Level1ShortCapacity;
	const int32 LongCapacity = Profile.IsImportant()
		? FMath::Clamp(Profile.LongMemoryCapacity > 0 ? Profile.LongMemoryCapacity : ZLSocialMemoryLimits::ImportantLongCapacity, 1, ZLSocialMemoryLimits::MaxLongCapacity)
		: 0;
	States.Add(Profile.AgentId, FZLSocialAgentState(ShortCapacity, LongCapacity));
	DecisionHistories.Add(Profile.AgentId);
	return true;
}

bool FZLSocialSimulation::UpdateAgentPosition(const FName AgentId, const FVector& Position)
{
	FZLSocialAgentProfile* Profile = Profiles.Find(AgentId);
	if (Profile == nullptr || !Router.UpdateAgentPosition(AgentId, Position)) { return false; }
	Profile->Position = Position;
	return true;
}

bool FZLSocialSimulation::UnregisterAgent(const FName AgentId)
{
	if (!Router.UnregisterAgent(AgentId)) { return false; }
	if (const FZLSocialAgentState* ExistingState = States.Find(AgentId))
	{
		LongMemoryItems = FMath::Max(0, LongMemoryItems - ExistingState->LongMemory.Num());
	}
	Profiles.Remove(AgentId);
	States.Remove(AgentId);
	DecisionHistories.Remove(AgentId);
	return true;
}

bool FZLSocialSimulation::CreateEvent(const FGameplayTag Type, const FName SourceId, const FName TargetId, const FVector& Position, const double NowSeconds, FZLSocialEvent& OutEvent) const
{
	return Router.CreateEvent(Type, SourceId, TargetId, Position, NowSeconds, OutEvent);
}

bool FZLSocialSimulation::ConfirmReport(const FZLSocialReportConfirmation& Confirmation, FZLSocialPropagationResult& OutResult)
{
	const bool bConfirmed = Propagation.ConfirmReport(Confirmation, OutResult);
	if (bConfirmed) { PropagationCreated += OutResult.DerivedEvents.Num(); }
	else { ++PropagationRejected; }
	PropagationRejected += OutResult.RejectedReceivers;
	return bConfirmed;
}

bool FZLSocialSimulation::ConfirmFactionStanding(const FZLSocialEvent& Event, const FName AuthorityId, const FZLSocialPerceptionResult& Perception, const double NowSeconds)
{
	const FZLSocialAgentProfile* Authority = Profiles.Find(AuthorityId);
	return Authority != nullptr && RelationshipStore.ConfirmFactionStanding(Event, *Authority, Perception, NowSeconds);
}

bool FZLSocialSimulation::ProcessEvent(const FZLSocialEvent& Event, const double NowSeconds, TArray<FZLSocialIntentCommand>& OutCommands, FZLSocialProcessingStats& OutStats, const TFunctionRef<bool(const FVector&, const FVector&)> HasLineOfSight)
{
	const double StartedAt = FPlatformTime::Seconds();
	OutCommands.Reset();
	OutStats = FZLSocialProcessingStats();
	FZLSocialEventRouteResult RouteResult;
	if (!Router.RouteEvent(Event, NowSeconds, RouteResult)) { return false; }
	OutStats.Spatial = RouteResult.SpatialStats;
	RootDuplicates += RouteResult.DuplicateCount;

	for (const FZLSocialAgentProfile& Agent : RouteResult.Candidates)
	{
		const FZLSocialPerceptionResult Perception = PerceptionFilter.Evaluate(Event, Agent, NowSeconds, HasLineOfSight);
		if (!Perception.bPerceived) { continue; }
		++OutStats.PerceivedAgents;
		FZLSocialAgentState& State = States.FindChecked(Agent.AgentId);
		FZLSocialRelationshipDelta RelationshipDelta;
		RelationshipStore.ApplyPersonalEvent(Event, Agent, Perception, NowSeconds, &RelationshipDelta);
		const float RelationshipImpact = FMath::Clamp(FMath::Abs(RelationshipDelta.Trust) + FMath::Abs(RelationshipDelta.Affinity) + FMath::Abs(RelationshipDelta.Fear) + FMath::Abs(RelationshipDelta.Reputation), 0.0f, 1.0f);
		const FZLSocialAgentProfile* Subject = Profiles.Find(Event.SourceId);
		const int32 LongMemoryBefore = State.LongMemory.Num();
		State.ApplyPerception(Event, Perception, Agent.Personality, RelationshipImpact, Subject != nullptr ? Subject->FactionId : NAME_None);
		LongMemoryItems += State.LongMemory.Num() - LongMemoryBefore;
		FZLSocialDecisionHistory& History = DecisionHistories.FindChecked(Agent.AgentId);
		FZLSocialDecisionContext Context;
		if (const FZLSocialRelationshipState* Relationship = RelationshipStore.FindRelationship(Agent.AgentId, Event.SourceId))
		{
			Context.Relationship = *Relationship;
			Context.bHasRelationship = true;
		}
		if (!Agent.FactionId.IsNone())
		{
			if (const FZLSocialFactionStandingState* Standing = RelationshipStore.FindFactionStanding(Agent.FactionId, Event.SourceId))
			{
				Context.FactionStanding = Standing->Standing;
				Context.bHasFactionStanding = true;
			}
		}
		FZLSocialLongMemoryQuery MemoryQuery;
		MemoryQuery.SubjectId = Event.SourceId;
		MemoryQuery.TopK = ZLSocialMemoryLimits::MaxTopK;
		const TArray<FZLSocialMemoryEntry> RelevantMemory = State.LongMemory.Retrieve(MemoryQuery, NowSeconds);
		Context.RelevantMemoryCount = RelevantMemory.Num();
		for (const FZLSocialMemoryEntry& Entry : RelevantMemory) { Context.StrongestMemoryImportance = FMath::Max(Context.StrongestMemoryImportance, Entry.Importance); }
		Context.SourceConfidence = Perception.EffectiveIntensity;
		Context.OccupationId = Agent.OccupationId;
		Context.bAlreadyReportedRoot = Propagation.HasReporterReported(Event.RootEventId, Agent.AgentId);
		const double RuleStartedAt = FPlatformTime::Seconds();
		FZLSocialDecisionResult Decision = DecisionEngine.Evaluate(Event, Agent, State.Instant, Perception, NowSeconds, History, Context);
		OutStats.RuleEvaluationMilliseconds += (FPlatformTime::Seconds() - RuleStartedAt) * 1000.0;
		++OutStats.RuleEvaluations;
		FZLSocialIntentCommand& Command = OutCommands.AddDefaulted_GetRef();
		Command.EventId = Event.EventId;
		Command.RootEventId = Event.RootEventId;
		Command.ParentEventId = Event.ParentEventId;
		Command.AgentId = Agent.AgentId;
		Command.SubjectId = Event.SourceId;
		Command.Intent = Decision.Intent;
		Command.SourceChannel = Perception.Channel;
		Command.SourceConfidence = Perception.EffectiveIntensity;
		Command.ChainDepth = Event.ChainDepth;
		Command.ChainBudget = Event.ChainBudget;
		Command.CandidateScores = MoveTemp(Decision.Candidates);
		Command.ReasonCodes = MoveTemp(Decision.ReasonCodes);
		LastCommands.Add(Agent.AgentId, Command);
	}
	OutStats.ProcessingMilliseconds = (FPlatformTime::Seconds() - StartedAt) * 1000.0;
	OutStats.Aggregate = GetAggregateMetrics();
	return true;
}

void FZLSocialSimulation::DecayAgentStates(const float DeltaSeconds)
{
	for (TPair<FName, FZLSocialAgentState>& Pair : States) { Pair.Value.Decay(DeltaSeconds); }
	RelationshipStore.DecayTowardsNeutral(DeltaSeconds);
}

void FZLSocialSimulation::Reset()
{
	Router.Reset(); Propagation.Reset(); RelationshipStore.Reset(); Profiles.Reset(); States.Reset(); DecisionHistories.Reset(); LastCommands.Reset();
	PropagationCreated = 0; PropagationRejected = 0; RootDuplicates = 0; LongMemoryItems = 0;
}

bool FZLSocialSimulation::BuildDebugSnapshot(const FName AgentId, FZLSocialAgentDebugSnapshot& OutSnapshot) const
{
	const FZLSocialAgentProfile* Profile = Profiles.Find(AgentId);
	const FZLSocialAgentState* State = States.Find(AgentId);
	if (Profile == nullptr || State == nullptr) { return false; }
	OutSnapshot = FZLSocialAgentDebugSnapshot();
	OutSnapshot.AgentId = AgentId;
	OutSnapshot.Personality = Profile->Personality;
	OutSnapshot.InstantState = State->Instant;
	OutSnapshot.ShortMemory = State->ShortMemory.GetChronological();
	if (const FZLSocialIntentCommand* Command = LastCommands.Find(AgentId))
	{
		OutSnapshot.LastEventId = Command->EventId;
		OutSnapshot.RootEventId = Command->RootEventId;
		OutSnapshot.ParentEventId = Command->ParentEventId;
		OutSnapshot.SubjectId = Command->SubjectId;
		OutSnapshot.ChainDepth = Command->ChainDepth;
		OutSnapshot.ChainBudget = Command->ChainBudget;
		OutSnapshot.SourceChannel = Command->SourceChannel;
		OutSnapshot.SourceConfidence = Command->SourceConfidence;
		OutSnapshot.FinalIntent = Command->Intent;
		OutSnapshot.CandidateScores = Command->CandidateScores;
		OutSnapshot.ReasonCodes = Command->ReasonCodes;
		if (const FZLSocialRelationshipState* Relationship = RelationshipStore.FindRelationship(AgentId, Command->SubjectId))
		{
			OutSnapshot.Relationship = *Relationship;
			OutSnapshot.bHasRelationship = true;
		}
		if (!Profile->FactionId.IsNone())
		{
			if (const FZLSocialFactionStandingState* Standing = RelationshipStore.FindFactionStanding(Profile->FactionId, Command->SubjectId))
			{
				OutSnapshot.FactionStanding = *Standing;
				OutSnapshot.bHasFactionStanding = true;
			}
		}
	}
	OutSnapshot.AgentLevel = Profile->AgentLevel;
	OutSnapshot.FactionId = Profile->FactionId;
	OutSnapshot.OccupationId = Profile->OccupationId;
	OutSnapshot.LongMemory = State->LongMemory.GetEntries();
	return true;
}

const FZLSocialAgentState* FZLSocialSimulation::FindAgentState(const FName AgentId) const
{
	return States.Find(AgentId);
}

FZLSocialAggregateMetrics FZLSocialSimulation::GetAggregateMetrics() const
{
	FZLSocialAggregateMetrics Metrics;
	Metrics.PropagationCreated = PropagationCreated;
	Metrics.PropagationRejected = PropagationRejected;
	Metrics.RootDuplicates = RootDuplicates;
	Metrics.RelationshipEdges = RelationshipStore.GetRelationshipEdgeCount();
	Metrics.FactionStandings = RelationshipStore.GetFactionStandingCount();
	Metrics.LongMemoryItems = LongMemoryItems;
	return Metrics;
}
