#include "Misc/AutomationTest.h"
#include "ZLSocialDecision.h"
#include "ZLSocialTags.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	FGameplayTag DecidePunch(const FZLSocialPersonalityTraits& Personality, const FZLSocialInstantState& State)
	{
		FZLSocialAgentProfile Agent;
		Agent.AgentId = TEXT("agent");
		Agent.Personality = Personality;
		FZLSocialEvent Event;
		Event.Type = ZLSocialTags::Event_Punch;
		FZLSocialPerceptionResult Perception;
		Perception.bPerceived = true;
		FZLSocialDecisionHistory History;
		return FZLSocialRuleDecisionEngine().Evaluate(Event, Agent, State, Perception, 1.0, History).Intent;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialDecisionTest, "ZL.Social.Decision.PersonalityCooldownAndExtreme", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialDecisionTest::RunTest(const FString& Parameters)
{
	FZLSocialPersonalityTraits Observer;
	Observer.Brave = Observer.FearSensitivity = Observer.Justice = Observer.Aggression = Observer.Social = 0.0f;
	Observer.Curiosity = 1.0f;
	FZLSocialInstantState Calm;
	Calm.Fear = 0.1f;
	const FGameplayTag FirstObserverDecision = DecidePunch(Observer, Calm);
	TestTrue(TEXT("Curious personality observes"), FirstObserverDecision == ZLSocialTags::Intent_Observe);
	TestTrue(TEXT("Identical input repeats deterministically"), DecidePunch(Observer, Calm) == FirstObserverDecision);

	FZLSocialPersonalityTraits Fearful = Observer;
	Fearful.Curiosity = 0.0f; Fearful.FearSensitivity = 1.0f;
	FZLSocialInstantState Afraid; Afraid.Fear = 0.8f;
	TestTrue(TEXT("Fearful personality flees"), DecidePunch(Fearful, Afraid) == ZLSocialTags::Intent_Flee);

	FZLSocialPersonalityTraits Reporter = Observer;
	Reporter.Curiosity = 0.0f; Reporter.Justice = 1.0f; Reporter.Social = 1.0f;
	TestTrue(TEXT("Social just personality reports"), DecidePunch(Reporter, Calm) == ZLSocialTags::Intent_Report);

	FZLSocialPersonalityTraits Fighter = Observer;
	Fighter.Curiosity = 0.0f; Fighter.Brave = 1.0f; Fighter.Aggression = 1.0f;
	FZLSocialInstantState Angry; Angry.Fear = 0.1f; Angry.Anger = 0.8f;
	TestTrue(TEXT("Brave aggressive personality confronts"), DecidePunch(Fighter, Angry) == ZLSocialTags::Intent_Confront);

	FZLSocialAgentProfile Agent; Agent.AgentId = TEXT("agent"); Agent.Personality = Observer;
	FZLSocialPerceptionResult Perception; Perception.bPerceived = true;
	FZLSocialEvent Punch; Punch.Type = ZLSocialTags::Event_Punch;
	FZLSocialDecisionHistory History;
	const FZLSocialRuleDecisionEngine Engine;
	TestTrue(TEXT("Initial decision is observe"), Engine.Evaluate(Punch, Agent, Calm, Perception, 1.0, History).Intent == ZLSocialTags::Intent_Observe);
	Agent.Personality = Fearful;
	TestTrue(TEXT("Cooldown prevents immediate switching"), Engine.Evaluate(Punch, Agent, Afraid, Perception, 1.1, History).bHeldByCooldown);
	FZLSocialDecisionHistory HysteresisHistory;
	const FZLSocialRuleDecisionEngine HysteresisEngine(0.0f, 0.2f);
	Agent.Personality = Observer;
	HysteresisEngine.Evaluate(Punch, Agent, Calm, Perception, 1.0, HysteresisHistory);
	Agent.Personality = Reporter;
	TestTrue(TEXT("Hysteresis rejects a marginal same-priority switch"), HysteresisEngine.Evaluate(Punch, Agent, Calm, Perception, 2.0, HysteresisHistory).bHeldByHysteresis);
	FZLSocialEvent Gunshot; Gunshot.Type = ZLSocialTags::Event_Gunshot;
	Agent.Personality = Fearful;
	const FZLSocialDecisionResult Extreme = Engine.Evaluate(Gunshot, Agent, Afraid, Perception, 1.2, History);
	TestTrue(TEXT("Extreme event overrides curiosity and cooldown"), Extreme.Intent == ZLSocialTags::Intent_Flee);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialHistoryAwareDecisionTest, "ZL.Social.Decision.RelationshipMemoryFactionHistory", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialHistoryAwareDecisionTest::RunTest(const FString& Parameters)
{
	FZLSocialAgentProfile Guard;
	Guard.AgentId = TEXT("guard");
	Guard.AgentLevel = EZLSocialAgentLevel::Important;
	Guard.OccupationId = TEXT("guard");
	Guard.Personality.Brave = 0.5f;
	Guard.Personality.FearSensitivity = 0.2f;
	Guard.Personality.Curiosity = 0.5f;
	Guard.Personality.Justice = 0.6f;
	Guard.Personality.Aggression = 0.4f;
	Guard.Personality.Social = 0.4f;
	FZLSocialInstantState State;
	State.Fear = 0.1f;
	State.Anger = 0.2f;
	State.Alert = 0.4f;
	FZLSocialEvent Punch;
	Punch.Type = ZLSocialTags::Event_Punch;
	FZLSocialPerceptionResult Perception;
	Perception.bPerceived = true;
	Perception.Channel = EZLSocialPerceptionChannel::Social;
	Perception.EffectiveIntensity = 0.7f;
	const FZLSocialRuleDecisionEngine Engine(0.0f, 0.0f);

	FZLSocialDecisionContext Neutral;
	Neutral.OccupationId = Guard.OccupationId;
	Neutral.SourceConfidence = 0.7f;
	Neutral.RelevantMemoryCount = 2;
	Neutral.StrongestMemoryImportance = 0.7f;
	FZLSocialDecisionHistory NeutralHistory;
	const FZLSocialDecisionResult Investigate = Engine.Evaluate(Punch, Guard, State, Perception, 1.0, NeutralHistory, Neutral);
	TestTrue(TEXT("Uncertain guard investigates"), Investigate.Intent == ZLSocialTags::Intent_Investigate);
	TestTrue(TEXT("Decision exposes stable reason codes"), Investigate.ReasonCodes.Contains(TEXT("source.uncertainty")) && Investigate.ReasonCodes.Contains(TEXT("occupation.guard")));

	FZLSocialDecisionContext Trusted = Neutral;
	Trusted.bHasRelationship = true;
	Trusted.Relationship.Trust = 1.0f;
	Trusted.Relationship.Affinity = 1.0f;
	Trusted.Relationship.Reputation = 1.0f;
	FZLSocialDecisionHistory TrustedHistory;
	const FZLSocialDecisionResult Assist = Engine.Evaluate(Punch, Guard, State, Perception, 1.0, TrustedHistory, Trusted);
	TestTrue(TEXT("Trusted history produces assist"), Assist.Intent == ZLSocialTags::Intent_Assist);

	FZLSocialDecisionContext Hostile = Neutral;
	Hostile.bHasRelationship = true;
	Hostile.Relationship.Trust = -1.0f;
	Hostile.Relationship.Affinity = -1.0f;
	Hostile.Relationship.Fear = 1.0f;
	Hostile.Relationship.Reputation = -1.0f;
	Hostile.bHasFactionStanding = true;
	Hostile.FactionStanding = -1.0f;
	FZLSocialDecisionHistory HostileHistory;
	const FZLSocialDecisionResult Confront = Engine.Evaluate(Punch, Guard, State, Perception, 1.0, HostileHistory, Hostile);
	TestTrue(TEXT("Hostile history produces confront"), Confront.Intent == ZLSocialTags::Intent_Confront);
	TestTrue(TEXT("Hostile reason includes relationship"), Confront.ReasonCodes.Contains(TEXT("relationship.threat")));

	FZLSocialDecisionHistory RepeatHistory;
	const FZLSocialDecisionResult Repeat = Engine.Evaluate(Punch, Guard, State, Perception, 1.0, RepeatHistory, Hostile);
	TestTrue(TEXT("Same history is deterministic"), Repeat.Intent == Confront.Intent && Repeat.ReasonCodes == Confront.ReasonCodes);

	FZLSocialDecisionContext AlreadyReported = Neutral;
	AlreadyReported.bAlreadyReportedRoot = true;
	FZLSocialDecisionHistory ReportHistory;
	const FZLSocialDecisionResult NoDuplicateReport = Engine.Evaluate(Punch, Guard, State, Perception, 1.0, ReportHistory, AlreadyReported);
	TestTrue(TEXT("Confirmed root cannot select report again"), NoDuplicateReport.Intent != ZLSocialTags::Intent_Report);
	return true;
}

#endif
