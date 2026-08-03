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
	TestTrue(TEXT("Curious personality observes"), DecidePunch(Observer, Calm) == ZLSocialTags::Intent_Observe);

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
	FZLSocialEvent Gunshot; Gunshot.Type = ZLSocialTags::Event_Gunshot;
	const FZLSocialDecisionResult Extreme = Engine.Evaluate(Gunshot, Agent, Afraid, Perception, 1.2, History);
	TestTrue(TEXT("Extreme event overrides curiosity and cooldown"), Extreme.Intent == ZLSocialTags::Intent_Flee);
	return true;
}

#endif
