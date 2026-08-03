#include "Misc/AutomationTest.h"
#include "ZLSocialPerception.h"
#include "ZLSocialTags.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialPerceptionTest, "ZL.Social.Perception.ChannelsAndBounds", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialPerceptionTest::RunTest(const FString& Parameters)
{
	FZLSocialEvent Event;
	Event.EventId = FGuid::NewGuid();
	Event.Type = ZLSocialTags::Event_Punch;
	Event.Position = FVector::ZeroVector;
	Event.Radius = 1000.0f;
	Event.Severity = 0.8f;
	Event.Noise = 0.4f;
	Event.Channels = static_cast<int32>(EZLSocialPerceptionChannel::Visual) | static_cast<int32>(EZLSocialPerceptionChannel::Auditory);
	Event.CreatedAtSeconds = 1.0;
	Event.ExpiresAtSeconds = 3.0;

	FZLSocialAgentProfile Agent;
	Agent.AgentId = TEXT("observer");
	Agent.Position = FVector(500.0f, 0.0f, 0.0f);
	const FZLSocialPerceptionFilter Filter;
	const auto Visible = [](const FVector&, const FVector&) { return true; };
	const auto Occluded = [](const FVector&, const FVector&) { return false; };

	FZLSocialPerceptionResult Result = Filter.Evaluate(Event, Agent, 2.0, Visible);
	TestTrue(TEXT("Visible nearby event is perceived"), Result.bPerceived);
	TestEqual(TEXT("Visual is the strongest channel"), Result.Channel, EZLSocialPerceptionChannel::Visual);
	Result = Filter.Evaluate(Event, Agent, 2.0, Occluded);
	TestTrue(TEXT("Auditory remains available when occluded"), Result.bPerceived);
	TestEqual(TEXT("Occluded event uses auditory"), Result.Channel, EZLSocialPerceptionChannel::Auditory);

	Agent.Position = FVector(1001.0f, 0.0f, 0.0f);
	TestFalse(TEXT("Outside radius is rejected"), Filter.Evaluate(Event, Agent, 2.0, Visible).bPerceived);
	TestFalse(TEXT("Expired event is rejected"), Filter.Evaluate(Event, Agent, 4.0, Visible).bPerceived);

	Event.TargetId = Agent.AgentId;
	Event.Channels |= static_cast<int32>(EZLSocialPerceptionChannel::Direct);
	Result = Filter.Evaluate(Event, Agent, 2.0, Occluded);
	TestTrue(TEXT("Direct target bypasses radius and occlusion"), Result.bPerceived);
	TestEqual(TEXT("Direct channel wins"), Result.Channel, EZLSocialPerceptionChannel::Direct);
	return true;
}

#endif
