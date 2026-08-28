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
	return Propagation.ConfirmReport(Confirmation, OutResult);
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
		State.ApplyPerception(Event, Perception, Agent.Personality, RelationshipImpact, Subject != nullptr ? Subject->FactionId : NAME_None);
		FZLSocialDecisionHistory& History = DecisionHistories.FindChecked(Agent.AgentId);
		FZLSocialDecisionResult Decision = DecisionEngine.Evaluate(Event, Agent, State.Instant, Perception, NowSeconds, History);
		++OutStats.RuleEvaluations;
		FZLSocialIntentCommand& Command = OutCommands.AddDefaulted_GetRef();
		Command.EventId = Event.EventId;
		Command.AgentId = Agent.AgentId;
		Command.Intent = Decision.Intent;
		Command.CandidateScores = MoveTemp(Decision.Candidates);
		LastCommands.Add(Agent.AgentId, Command);
	}
	OutStats.ProcessingMilliseconds = (FPlatformTime::Seconds() - StartedAt) * 1000.0;
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
		OutSnapshot.FinalIntent = Command->Intent;
		OutSnapshot.CandidateScores = Command->CandidateScores;
	}
	return true;
}

const FZLSocialAgentState* FZLSocialSimulation::FindAgentState(const FName AgentId) const
{
	return States.Find(AgentId);
}
